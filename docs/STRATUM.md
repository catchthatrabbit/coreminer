# CoreMiner Stratum Protocol

## Connection Format

Pool connections are defined using the `-P` argument with the following syntax:

```bash
-P scheme://user[.workername][:password]@hostname:port[/...]
```

> **Note**: Values in square brackets are optional.

Supported schemes:

- `http` - getwork mode
- `stratum+tcp` - plain stratum mode
- `stratum1+tcp` - plain stratum core-proxy compatible mode
- `stratum2+tcp` - plain stratum NiceHash compatible mode

## URI Notation

This notation provides flexibility by allowing CoreMiner to specify all needed arguments per connection. Other miners typically use dedicated CLI arguments that apply to all connections.

A URI is structured as follows:

```txt
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

You can optionally append additional information as a path:

```txt
stratum://cb57bbbb54cdf60fa666fd741be78f794d4608d67109.worker-01:password@eu.catchthatrabbit.com:8008/something/else
                                                                                                     +--------------+
                                                                                                       |
                                                                                  Path --------------- +
```

> **Note**: Any characters in the Path section must be URL encoded. For example, `@` must be written as `%40`.

## Special Characters

Due to pool compatibility, we need specific delimiters:

- `.` (dot) separates the account from the workername
- `:` (colon) separates the workername from the password

If your values contain these characters or others that might affect parsing, you have two options:

1. Enclose the string in backticks (ASCII 96)
2. URL encode the problematic characters

Example with an account name containing a dot:

```bash
# Using backticks
-P stratum://`account.1234`.worker-01:password@eu.catchthatrabbit.com:8008

# Using URL encoding
-P stratum://account%2e1234.worker-01:password@eu.catchthatrabbit.com:8008
```

> **Note**: On Unix-like systems, backticks have special meaning for command execution. You may need to escape them:

```bash
-P stratum://\`account.1234\`.worker-01:password@eu.catchthatrabbit.com:8008
```

> **Note**: On Windows, the `%` symbol has special meaning in batch files. You may need to double it:

```batch
-P stratum://account%%2e1234.worker-01:password@eu.catchthatrabbit.com:8008
```

## Secure Socket Communications

CoreMiner supports secure socket communications to prevent [man-in-the-middle attacks](https://en.wikipedia.org/wiki/Man-in-the-middle_attack). To enable it, replace `tcp` with:

- `tls` - Enable secure socket communication
- `ssl` or `tls12` - Enable secure socket communication with TLS 1.2 only

Example:

```bash
-P stratum+tls://[...]
# or
-P stratum+tls12://[...]
```

This applies to all stratum variants (`stratum1` and `stratum2`).

## URL Encoding Reference

Common special characters and their URL encodings:

| Code | Character |
|:----:|:---------:|
| %25  | %         |
| %26  | &         |
| %2e  | .         |
| %2f  | /         |
| %3a  | :         |
| %3f  | ?         |
| %40  | @         |

## Automatic Stratum Detection

CoreMiner includes automatic stratum detection to simplify connection setup. Instead of guessing the correct stratum variant, you can use:

- `stratum://` - For automatic detection of plain stratum
- `stratums://` - For automatic detection of secure stratum
- `stratumss://` - For automatic detection of TLS 1.2 secure stratum
