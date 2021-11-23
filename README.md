# HTML Linker

It's essentially the concept of a linker for object files into one self-contained executable, but for HTML. It (_somewhat_ reliably) inlines external javascript files and CSS stylesheets, embedding them in the HTML to make it a single self-contained file that can be copy-pasted and/or embedded anywhere else. It's still incredibly buggy, though, and doesn't really understand comments, but works for my current use case. I might come back to it later and rewrite it with an actual lexer and some good practice.

## Quickstart

```
./bin/htmll ./test/src/index.html -o ./test/out/out.html
```

Should 'link' the HTML from the singular test case into a file called out.html in directory test/out.

## Command line options

- `htmll`: the name of the executable.
- `foo/bar.html`: the input file.
- `-o baz/bar.html`: the output file.
- `-v` | `--verbose`: verbose mode (for debugging purposes).
