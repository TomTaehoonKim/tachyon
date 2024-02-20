use std::marker::PhantomData;

use halo2_proofs::{
    arithmetic::FieldExt,
    circuit::{Layouter, SimpleFloorPlanner, Value},
    plonk::{Advice, Circuit, Column, ConstraintSystem, Error, Expression, Selector, TableColumn},
    poly::Rotation,
};

#[derive(Clone, Default)]
struct SimpleLookupCircuit<F: FieldExt> {
    _marker: PhantomData<F>,
}

#[derive(Clone)]
struct SimpleLookupConfig {
    selector: Selector,
    table: TableColumn,
    advice: Column<Advice>,
}

impl<F: FieldExt> Circuit<F> for SimpleLookupCircuit<F> {
    type Config = SimpleLookupConfig;
    type FloorPlanner = SimpleFloorPlanner;

    fn without_witnesses(&self) -> Self {
        Self::default()
    }

    fn configure(meta: &mut ConstraintSystem<F>) -> SimpleLookupConfig {
        let config = SimpleLookupConfig {
            selector: meta.complex_selector(),
            table: meta.lookup_table_column(),
            advice: meta.advice_column(),
        };

        meta.lookup("lookup", |meta| {
            let selector = meta.query_selector(config.selector);
            let not_selector = Expression::Constant(F::one()) - selector.clone();
            let advice = meta.query_advice(config.advice, Rotation::cur());
            vec![(selector * advice + not_selector, config.table)]
        });

        config
    }

    fn synthesize(
        &self,
        config: SimpleLookupConfig,
        mut layouter: impl Layouter<F>,
    ) -> Result<(), Error> {
        layouter.assign_table(
            || "3-bit table",
            |mut table| {
                for row in 0u64..(1 << 3) {
                    table.assign_cell(
                        || format!("row {}", row),
                        config.table,
                        row as usize,
                        || Value::known(F::from(row + 1)),
                    )?;
                }

                Ok(())
            },
        )?;

        layouter.assign_region(
            || "assign values",
            |mut region| {
                for offset in 0u64..(1 << 4) {
                    config.selector.enable(&mut region, offset as usize)?;
                    region.assign_advice(
                        || format!("offset {}", offset),
                        config.advice,
                        offset as usize,
                        || Value::known(F::from((offset % 8) + 1)),
                    )?;
                }

                Ok(())
            },
        )
    }
}

#[cfg(test)]
mod test {
    use std::marker::PhantomData;

    use crate::bn254::{
        Blake2bWrite as TachyonBlake2bWrite, ProvingKey as TachyonProvingKey, SHPlonkProver,
    };
    use crate::circuits::simple_lookup_circuit::SimpleLookupCircuit;
    use crate::consts::{TranscriptType, SEED};
    use crate::prover::create_proof as tachyon_create_proof;
    use crate::xor_shift_rng::XORShiftRng;
    use halo2_proofs::{
        plonk::keygen_pk2,
        poly::kzg::{
            commitment::{KZGCommitmentScheme, ParamsKZG},
            multiopen::ProverSHPLONK,
        },
        transcript::{Blake2bWrite, Challenge255, TranscriptWriterBuffer},
    };
    use halo2curves::bn256::{Bn256, Fr, G1Affine};
    use rand_core::SeedableRng;

    #[test]
    fn test_create_proof() {
        // ANCHOR: test-circuit
        // The number of rows in our circuit cannot exceed 2ᵏ. Since our example
        // circuit is very small, we can pick a very small value here.
        let k = 5;

        // Instantiate the circuit.
        let circuit = SimpleLookupCircuit::<Fr> {
            _marker: PhantomData,
        };

        // Arrange the public input.
        let public_inputs = vec![];
        let public_inputs2 = vec![&public_inputs[..]];

        // Given the correct public input, our circuit will verify.
        let s = Fr::from(2);
        let params = ParamsKZG::<Bn256>::unsafe_setup_with_s(k, s.clone());
        let pk = keygen_pk2(&params, &circuit).expect("vk should not fail");

        let rng = XORShiftRng::from_seed(SEED);

        let halo2_proof = {
            let mut transcript = Blake2bWrite::<_, G1Affine, Challenge255<_>>::init(vec![]);

            halo2_proofs::plonk::create_proof::<
                KZGCommitmentScheme<Bn256>,
                ProverSHPLONK<_>,
                _,
                _,
                _,
                _,
            >(
                &params,
                &pk,
                &[circuit.clone()],
                public_inputs2.as_slice(),
                rng.clone(),
                &mut transcript,
            )
            .expect("proof generation should not fail");

            transcript.finalize()
        };

        let tachyon_proof = {
            let mut prover = SHPlonkProver::new(TranscriptType::Blake2b as u8, k, &s);

            let mut pk_bytes: Vec<u8> = vec![];
            pk.write(&mut pk_bytes, halo2_proofs::SerdeFormat::RawBytesUnchecked)
                .unwrap();
            let mut tachyon_pk = TachyonProvingKey::from(pk_bytes.as_slice());
            let mut transcript = TachyonBlake2bWrite::init(vec![]);

            tachyon_create_proof::<_, _>(
                &mut prover,
                &mut tachyon_pk,
                &[circuit],
                public_inputs2.as_slice(),
                rng,
                &mut transcript,
            )
            .expect("proof generation should not fail");

            let mut proof = transcript.finalize();
            let proof_last = prover.get_proof();
            proof.extend_from_slice(&proof_last);
            proof
        };
        assert_eq!(halo2_proof, tachyon_proof);
        // ANCHOR_END: test-circuit
    }
}
