# hosts

Allow user-defined hosts entries.

`hosts` is a `LD_PRELOAD` library that overrides the fetching of hostnames in applications
with functionality that fetches these entries from a user-defined hosts file instead of `/etc/hosts`.

By default `hosts` fetches hosts entries from `~/.hosts`, but `HOSTS_FILE` can be used to specify a different file as well.

## Build

```sh
make
```

This results in `libhostspriv.so`.

## Install

```sh
mkdir ~/bin
cp libhostspriv.so ~/bin
echo 'export LD_PRELOAD=$HOME/bin/libhostspriv.so' >> ~/.bashrc
```

## Usage

Using the default hosts file in `$HOME`:

```sh
echo "127.0.0.1 somehost" >> ~/.hosts
nc -vz somehost 80
```

Using the `HOSTS_FILE` environment variable:

```sh
echo "127.0.0.1 somehost" >> hosts
HOSTS_FILE=$PWD/hosts nc -vz somehost 80
```

## Limitations

`hosts` does not work with suid programs. That includes `ping` unfortunately.
