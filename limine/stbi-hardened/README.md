# Hardened stb_image

A hardened version of [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h)
hardened with the help of LLMs (upstream has taken up a naive anti-LLM stance), static analysis,
fuzzing, and extensive testing.

While not a guarantee of security given the pervasive issues that upstream has and refuses
to fix, it is definitely a leap in the right direction.

A list of what issues were fixed compared to upstream can be found in the [FIXES.md](FIXES.md)
file.

Please report any additional security issues upstream, but if possible open a PR/issue in this
repository as well.
