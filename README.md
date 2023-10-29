# Mixxx augmented with Stems and Programmable AutoDJ

Mixxx is Free DJ software that gives you everything you need to perform live
DJ mixes. Mixxx works on GNU/Linux, Windows, and macOS.

This branch is a hacky-as-hell way to have Stems working with what I call programmable AutoDJ. Basically what we have is a hijacked AutoDJ callback, which via regular txt files communicates outside and inside towards Mixxx.

The file padj.py contains some Python logic to effectively steer Mixxx into mixing and particularly Stems mixing. The mixes are reproducible but this is hacky-as-hell - and that means buggy, specially if you make a mistake.

I tried to make the reference as agnostic as I could, text file communication should be supported on all platforms that Mixxx runs. You still have to have a separated/mdx_extra_q directory on your home directory loaded with the stems you want to use.

Remember to create the files you'll need - only the lock file is created automatically. For example, on a Linux shell:

```$ touch controlmixxx.txt```

```$ touch confirmixxx.txt```

```$ touch mixxxposition1.txt```

```$ touch mixxxposition2.txt```

```$ touch mixxxlooping1beatcounter.txt```

```$ touch mixxxlooping2beatcounter.txt```

Also, make sure to "zero-out" the relevant command and control txt files with the following commands:

```$ echo -ne "P10\n3\n" > controlmixxx.txt```

```$ echo -ne "-1\n" > confirmixxx.txt```

Remember to always "zero-out" in such a way every time you start Mixxx.

After that, you can safely turn on AutoDJ as usual, and subsequently run your own version of padj.py to program your Robot DJ :)

## License

Mixxx is released under the GPLv2. See the LICENSE file for a full copy of the
license.
