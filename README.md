# `clientside`

The goal of this work is to enable extremely fast client-side cryptography,
particulary ZK proof generation using elliptic curves over large prime fields.

The target platform is WASM.

There is much to be done:

- [x] Montgomery multiplication
    - [x] CIOS (Acar)
    - [x] CIOS (BM17)
    - [x] CIOS using f64s (Niall Emmart)
    - [ ] CIOS using 30-bit limbs (Mitscha-Baude)
- [ ] Multi-scalar multiplication
- [ ] Fast fourier transform
- [ ] Comparision against other libraries
- [ ] TBD

## Quick start

Ensure that you have `emcc` installed in your $PATH. Follow the [Emscripten
Tutorial](https://emscripten.org/docs/getting_started/Tutorial.html#tutorial)
to do this.

Also ensure that you have `node` installed in your $PATH.

run:

```bash
make
make run_tests
./run_benchmarks
```

## Credits

- Much of this code was adapted from [Niall Emmart's ZPrize 2023
submission](https://github.com/z-prize/2023-entries/tree/main/prize-2-msm-wasm/prize-2b-twisted-edwards/yrrid-snarkify).

- `c/sha256/` is from [LekKit/sha256](https://github.com/LekKit/sha256).

- `tests/minunit.h` is from [minunit](https://github.com/siu/minunit) and has a
  slight modification to allow it to compile to WASM.

## Miscellaneous

If you use Vim for development, as well as the ALE plugin, it helps to run
[bear](https://github.com/rizsotto/Bear):

```bash
bear -- make all
```

Which will generate `compile_commands.json`, and allow Vim and ALE to
preprocess and compile the source code before displaying any errors. Without
it, errors due to the preprocessor not being run by Vim/ALE will show up in
Vim.
