## Deployment

1.  Change version number in:
    - `CMakeList.txt`
    - `docs/man/ctune.1.md` (+ date)
    - `PKGBUILD`
2.  Check/change copyright years in:
    - `CMakeList.txt`
    - `docs/man/ctune.1.md`
3.  Clear build and CMake cache
4.  If change in functionality then modify as needed:
    - readme (`README.md`) 
    - man page (`docs/man/ctune.1.md`) + build the docs and commit new man file
    - PlantUML core and ui diagram (`docs/design/*`) + generate updated pics
5. Build