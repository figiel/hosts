# userhosts

Allow user-defined hosts entries.

`userhosts` is a `LD_PRELOAD` library that overrides the fetching of hostnames in applications. `userhosts` transforms hostnames using a user-defined hosts file before doing the actual lookup.

This allows using your own `userhosts` file on top of your system `/etc/hosts` file.

By default `userhosts` fetches hosts entries from the home directory (`~/.hosts`). This can be overridden using an environment variable (`HOSTS_FILE`) to specify a different file.

## Build

```sh
make
```

This results in `libuserhosts.so`.

## Install

You can choose to use `userhosts` globally for your user account by adding it to your shell, using bashrc:

```sh
mkdir ~/bin
cp libuserhosts.so ~/bin
echo 'export LD_PRELOAD=$HOME/bin/libuserhosts.so' >> ~/.bashrc

source ~/.bashrc
```

You can also use `userhosts` for individual commands:

```sh
LD_PRELOAD=$HOME/bin/libuserhosts.so firefox
```

## Usage

Using the default hosts file in `$HOME`:

```sh
echo "127.0.0.1 somehost" > ~/.hosts
nc -vz somehost 80
```

Using the `HOSTS_FILE` environment variable:

```sh
echo "127.0.0.1 somehost" > hosts
HOSTS_FILE=$PWD/hosts nc -vz somehost 80
```

Use hostnames as target to redirect name resolving:

```sh
echo "localhost somehost2" > ~/.hosts
nc -vz somehost2 80
```

Resolving will not happen recursively, so the following will attempt to DNS resolve `somehost`:

```sh
echo "localhost somehost" > ~/.hosts
echo "somehost somehost2" >> ~/.hosts
nc -vz somehost2 80
```

## Limitations

`userhosts` does not work with suid programs. That includes `ping` unfortunately.
