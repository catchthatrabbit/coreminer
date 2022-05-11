# Pool Examples for Core

Pool connection definition is issued via `-P` argument which has this syntax:

```
-P scheme://user[.workername][:password]@hostname:port[/...]
```
__values in square brackets are optional__

where `scheme` can be any of:

* `http` for getwork mode
* `stratum+tcp` for plain stratum mode
* `stratum1+tcp` for plain stratum core-proxy compatible mode
* `stratum2+tcp` for plain stratum NiceHash compatible mode

## A note about this form of notation
This notation is called URI notation and gives us great flexibility allowing Coreminer to specify all needed arguments per single connection (other miners offer single dedicated CLI arguments which are valid for all connections).
An URI is formed like this

```
                                   Authority
            +-----------------------------------------------------------------------------------------+
  stratum://cb57bbbb54cdf60fa666fd741be78f794d4608d67109.worker-01:password@eu.catchthatrabbit.com:8008
  +------+  +-------------------------------------------------------------+ +--------------------+ +--+
      |                         |                                  |                                |
      |                         |                                  |                                + > Port
      |                         |                                  + -------------------------------- > Host
      |                         + ------------------------------------------------------------------- > User Info
      + --------------------------------------------------------------------------------------------- > Scheme

```

Optionally you can append to the above notation anything which might be useful in the form of a path.
Example

```
stratum://cb57bbbb54cdf60fa666fd741be78f794d4608d67109.worker-01:password@eu.catchthatrabbit.com:8008/something/else
                                                                                                     +--------------+
                                                                                                       |
                                                                                  Path --------------- +
```

**Anything you put in the `Path` part must be Url Encoded thus, for example, `@` must be written as `%40`**

As you may have noticed due to compatibility with pools we need to know exactly which are the delimiters for the account, the workername (if any) and the password (if any) which are respectively a dot `.` and a column `:`.
Should your values contain any of the above mentioned chars or any other char which may impair the proper parsing of the URI you have two options:
- either enclose the string in backticks (ASCII 96)
- or URL encode the impairing chars

Say you need to provide the pool with an account name which contains a dot. At your discretion you may either write
```
-P stratum://`account.1234`.worker-01:password@eu.catchthatrabbit.com:8008
```  
or
```
-P stratum://account%2e1234.worker-01:password@eu.catchthatrabbit.com:8008
```  
The above samples produce the very same result.

**Backticks on *nix**. The backtick enclosure has a special meaning of execution thus you may need to further escape the sequence as
```
-P stratum://\`account.1234\`.worker-01:password@eu.catchthatrabbit.com:8008
```  
**`%` on Windows**. The percent symbol `%` has a special meaning in Windows batch files thus you may need to further escape it by doubling. Following example shows `%2e` needs to be replaced as `%%2e`
```
-P stratum://account%%2e1234.worker-01:password@eu.catchthatrabbit.com:8008
```  

## Secure socket communications for stratum only

Coreminer supports secure socket communications (where pool implements and offers it) to avoid the risk of a [man-in-the-middle attack](https://en.wikipedia.org/wiki/Man-in-the-middle_attack)
To enable it simply replace tcp with either:

* `tls` to enable secure socket communication
* `ssl` or `tls12` to enable secure socket communication **allowing only TLS 1.2** encryption

thus your connection scheme changes to `-P stratum+tls://[...]` or `-P stratum+tls12://[...]`. Same applies for `stratum1` and `stratum2`.

## Special characters in variables

You can use the %xx (xx=hexvalue of character) to pass special values.
Some examples:

| Code | Character |
| :---: |  :---: |
|%25 | % |
|%26 | & |
|%2e | . |
|%2f | / |
|%3a | : |
|%3f | ? |
|%40 | @ |

## Secure connections

Stratum autodetection has been introduced to mitigate user's duty to guess/find which stratum flavour to apply (stratum or stratum1 or stratum2).
If you want to let Coreminer do the tests for you simply enter scheme as `stratum://` (note `+tcp` is missing) or `stratums://` for secure socket or `stratumss://` for secure socket **allowing only TLS 1.2** encryption.
