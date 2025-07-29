# gosa2-container

This repository is used to hold the code
necessary to get GOsa² up and running in a container
environment.

It should be possible to run GOsa² on your local machine
and to get it work on your kubernetes environment as well.

## Current state

The state of container files is "work in progress".
The amount of options to use this as an basis to
setup an production environment are very limited.

Although you should find any interessting pieces here
to get your own environment up and running.

### What should work

1. Setup GOsa² with openLDAP >= 2.5
2. Use `START_TLS` for GOsa²s LDAP connection
3. Using a multi provider LDAP setup

### Setup

In the future it should be sufficient to apply a
manifest to your kubernetes environment with the
appropriate configuration values.

We do this to avoid the classic setup wizard that
still exists in GOsa².

#### Using docker compose

To get GOsa² up and running in a local container environment you need to do
the following steps:

1. clone this repository
2. create a `docker-compose.yml` file from the given example and fill out the necessary values
3. provide a valid ldap configuration
   1. a valid and working configuration is provided. The base DN is set to `dc=example,dc=com`
   2. rename `config.ldif.example` to `config.ldif`
   3. rename `data.ldif.example` to `data.ldif`
4. run: `docker compose build`
5. start all containers using `docker compose up` or `docker compose up -d`

After the start GOsa² Web Interface should be available at `http://localhost:8080/gosa` by default.
At this point you have a working GOsa². Congratulations!

ATTENTION: This example is not inteded to be used as a production environment. It just
serves a testing purpose. Keep in mind that we predefined some critical values like
passwords. If you choose to start a productive environment based on these files it
is crucial to change those values.