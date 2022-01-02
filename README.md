# HTML Linker

It's essentially the concept of a linker for object files into one static self-contained executable, but for HTML. 

It goes through an HTML source file, looks for `<script>` and `<link rel="stylesheet">` tags, and, when it finds one with a `src` or `href` attribute as a file, it nabs that file's contents and inlines the script/stylesheet in the HTML document. Test case in [./test/](./test/).

## Quick Start
```console
$ make all
$ ./bin/htmll ./test/src/index.html -o ./test/out/out.html
```

## Constraints
- Ending link tags are not supported. This should be easy enough to add in the future, but for now, link tags must end with ` />`.
- It does not understand URLs in script tags (or link tags, but it ignores those). All paths must be explicitly relative.
    - In the future, this may go grab contents from those URLs

## Notes
This project uses [my fork of tsoding's String_View library](https://github.com/minefreak19/sv).
