# `clientside`: Optimising Client-Side ZK in WASM

Privacy-preserving applications of zero-knowledge proofs typically rely on
protocols that use large-prime field and elliptic curve arithmetic. These
operations, however, are computationally intensive for consumer devices,
leading to latency and therefore subpar user experiences. Moreover, it is
unacceptable to outsource proof generation to a third-party server since this
would expose private user data. As such, it is important to optimise the
algorithms and techniques for client-side proof generation.

Significant progress in this area has emerged from the [ZPrize
competition](https://www.zprize.io) organised by [Aleo](https://aleo.org/) and
others. Submissions to this competition include accelerated code for [elliptic
curve and finite field
arithmetic](https://www.zprize.io/blog/announcing-zprize-results), as well as
speeding up the [multi-scalar multiplication
algorithm](https://www.zprize.io/blog/announcing-the-2023-zprize-winners) in
the browser using technologies such as WASM and
[WebGPU](https://en.wikipedia.org/wiki/WebGPU). All of this work has been
open-sourced, and anyone can adopt these optimisations.

Yet, there are still some gaps: some techniques are not fully documented, and
engineers need to look up various academic papers and dissect complicated
codebases in order to understand how they work. Furthermore, while benchmarks
of the contest submissions were performed by the ZPrize judges, benchmarks of
individual techniques, such as different ways to perform Montgomery
multiplication, have not yet been done.

The goal of this project is to address these issues. This document and
repository aims to help software engineers get up to speed with highly optmised
client-side cryptography. To start with, this document will provide detailed
explanations of optimised Montgomery multiplication algorithms for large prime
fields, as well as their associated benchmarks. In the future, this document
will cover more topics, such as but not limited to optimistions for elliptic
curve operations, multi-scalar multiplication, and fast Fourier transforms.
