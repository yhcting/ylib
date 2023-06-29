# ylib
## BUILD
run `configure-full.sh` to prepare build. And then run `make`

### Doxygen
tested on *doxygen* version 1.9.7.

## API Basic rules
### File
Header files for APIs MUST have prefix *y*.
And it's matching internal-file should have same name without prefix *y*.


### Naming rule
`<module-name>_<sub-module-name>_<sub-sub-module-name>_..._<action(verb)>_...`

- *module* means name of *main object*. This module SHOULD be the FIRST parameter of API functions except for APIs to create module-object.
```
    ex: module-name = 'ygraph'
    struct ygraph * ygraph_create(...);
    void ygraph_add_vertex(struct yhash *, ...);
    void ygraph_destroy(ygraph *);
```

- *sub-module* may or may not require main-module-object.
    - **A**: If main-module is required, main-module SHOULD be the first parameter.
    - **B**: If not, sub-module SHOULD be the first parameter.
    - If both APIs exist, **B** has priority. That is, `<module-name>_<sub-module-name>...` SHOULD be used only for **B** cases.
```
    ex: module-name = 'ygraph', sub-module-name = 'vertex'
    CASE (*A)
    ygraph_add_vertex_...(struct ygraph *, struct yvertex *, ...)
    ygraph_remove_vertex_...(struct ygraph *, struct yvertex *, ...)

    CASE (*B)
    ygraph_vertex_find_...(struct yvertex *, ...)
    ygraph_vertex_has_...(struct yvertex *, ...)
```

### Return value
- APIs having `int` return-type.
    - `<0` : Fail. And -errno is returned.
    - `>=0` : Success. 0 represents 'general success.'


### Parameters
- Pointer parameter used as 'input' MUST be 'const pointer'.
 (Therefore, non-const pointer parameter means 'in/out' or 'out'.)


### Non-api functions or macros
- Non-api functions/macros MUST have *YY* as symbol prefix.
    > Why this policy is required?
    Sometimes, 'static inline' APIs may be used due to simplicity and performance.
    In this case, functions or macros that SHOULD NOT be included as API may be used inside in them.
    And definition of them should be included at API headers.
    This may cause symbol-conflict when ylib is used in other program.


### Terms
- *add* <-> *remove*.
    > Adding/Removing new object to module.
    *add* SHOULD mean *passing object reference*(shallow copy).
    Term *delete* SHOULD NOT be used as couter-part of term *add*.
- *create* <-> *destroy*
    > *create* means make new object by allocating required resources for it.
- *alloc* <-> *free*
    > *alloc* is similar with *create*. But it is used for small module (ex. sub modules or structures.)


## TODO
- Re-design & implement build description (*Makefile*)
- Implement module for timer
- Implement module for stack heap
- *msghandler* should be able to be destroyed regardless of *msglooper*.
- Implement *collection* and *set* module - like python *set*, *list*, *dict*
