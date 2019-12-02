# wgetX

`wgetX` is a commandline download utility: it can be used to dowload webpages and other resources available on the web using the HTTP protocol. For example:

```console
$ ./wgetX "http://example.com/" "example.html"
The file has been saved as example.html.

$ cat "./example.html"
<!doctype html>
<html>
<head>
    <title>Example Domain</title>
    ... snip ...
```

## Usage

```
wgetX <URI> [output]
```

`wgetX` will download the file available at `<URI>` and save it in the file `[output]`. If no output name is given, it will default to `received_page`.

## Compiling
The compilation process requires no external libraries and can be simply initiated with

```console
$ make
```

This process will lead to the creation of the `wgetX` executable.

Debug builds can be compiled using

```
$ make BUILD=debug
```

The resulting executable will be called `wgetX.debug`.

## Additional notes

`wgetX` will follow HTTP redirects when it receives a status code in the 3xx range. There is no limit on the number of redirects `wgetX` will follow, and it may therefore end in a redirect loop.

`wgetX` should be able to reach resources over IPv4 and IPv6 ; it will try both in the order given by `getaddrinfo`.

## Credit

Code template by Jiazi Yi

> LIX, Ecole Polytechnique  
> <jiazi.yi@polytechnique.edu>

Template updated by Pierre Pfister

> Cisco Systems  
> <ppfister@cisco.com>

Code completed by Valentin Ogier
