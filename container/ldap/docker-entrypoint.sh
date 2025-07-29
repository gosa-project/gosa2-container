#!/bin/bash

set -e

command="$(command -v slapd)"

echo $LDAP_TYPE

find /etc/ldap/
if [ ! -f /etc/ldap/slapd.d/cn=config.ldif ]; then
	echo "Applying initial configuration"
	slapadd -l /etc/opt/init/config.ldif -b cn=config -F /etc/ldap/slapd.d
	slapadd -q -s -l /etc/opt/init/data.ldif -F /etc/ldap/slapd.d
fi

# if [ ! -f /var/lib/ldap/data.mdb ]; then
# 	xz --decompress --to-stdout /etc/opt/init/init.ldif.xz | slapadd -q -b dc=example,dc=de
# fi

exec /usr/sbin/slapd -h "ldap://$HOSTNAME ldaps://$HOSTNAME" -d 256
