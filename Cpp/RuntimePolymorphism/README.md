# Runtime Polymorphism

RuntimePolymorphism2.cpp is a simplification of RuntimePolymorphism1.cpp where in class Object the unique_ptr has been replaced by shared_ptr<const> in order to suppress copies. This also suppresses explicit copy/move constructor and copy/move operator.

Based on Sean Parent's "Better Code: Runtime Polymorphism" talk.
