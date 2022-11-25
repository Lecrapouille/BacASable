# Fesign Pattern Visitor

## 00_BasicVisitor

Initial basic vistor implementation.

## 01_FallbackVisitor

Based on 00_BasicVisitor with fallback management: behavior when unexpected elements are
detected when the visitot is traversing the container of vistable elements.

Methods `visit` have been replaced by `operator()`.
