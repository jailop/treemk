2025-11-24:

- Reviewing the approach to use an external library for preview markdown
  rendering. Currently, rendering is done using an ad hoc function, but
  it is incomplete and error-prone. Besides that, it is duplicating
  functionality that is already available in third-party solutions. The
  disadvantage of including a third-party library is that it would add an
  additional dependency to the project. It is to be noted that current
  issues are related to the markdown rendering feature.
- Evaluating the approach to initiate the testing suite. As the start
  point, the tests will include file management tasks, that is one of
  the core functionalities of the application. Other critical part is
  markdown rendering, but the testing support for that features will be
  considered until the decision about the external library is made.
