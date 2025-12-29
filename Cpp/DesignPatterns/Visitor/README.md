# Design Pattern Visitor

## 00_BasicVisitor

Initial basic visitor implementation.

## 01_FallbackVisitor

Based on 00_BasicVisitor with fallback management: behavior when unexpected elements are
detected when the visitor is traversing the container of visitable elements.

Methods `visit` have been replaced by `operator()`.
