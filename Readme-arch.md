Notes on arch in general:

* avoid tight coupling
* inheritance is tight coupling, don't overuse it (use it for is-a 
  relationships only)
* OOP concepts: encapsulation / data hiding. Polymorphism (runtime vs
  compile-time).
* example for compile-time polymorphism: std::sort
* example for runtime polymorphism: anything with inheritance, e.g. ios_base
* class invariants: make sure 'protected' does not undermine these
* goals: performance (speed), energy efficiency, availability, 
  productivity/maintainability, security, ops, license costs.
  -> trade-offs between non-functional requirements
* refactoring, technical debt
* sonar metrics: cyclomatic complexity

Links:
* http://www.slideshare.net/ewolff/practices-of-good-software-architects
* fowler

TODO:
* to read Pragmatic Programmer
* software craftsmanship
* fowler: Patterns of Enterprise Architecture
* (done: ReleaseIt!, ContinuousDeployment)
