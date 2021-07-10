# hosts

Allow user-defined hosts entries.

`hosts` is a `LD_PRELOAD` library that overrides the fetching of hostnames in applications. `hosts` transforms hostnames using a user-defined hosts file before doing the actual lookup.

This allows using your own `hosts` file on top of your system `/etc/hosts` file.

By default `hosts` fetches hosts entries from the home directory (`~/.hosts`). This can be overridden using an environment variable (`HOSTS_FILE`) to specify a different file.

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
