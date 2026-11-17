#!/bin/bash

cp /srv/certs/root_ca.pem /usr/local/share/ca-certificates/root_ca.crt
update-ca-certificates

# service nginx start

/opt/privacyidea/bin/pi-manage setup create_tables
/opt/privacyidea/bin/pi-manage setup create_enckey
/opt/privacyidea/bin/pi-manage setup create_audit_keys
/opt/privacyidea/bin/pi-manage admin add -p gosa-admin-secret gosa-admin
/opt/privacyidea/bin/pi-manage config resolver create gosa_ldap ldapresolver /etc/privacyidea/gosa_ldapresolver.conf
/opt/privacyidea/bin/pi-manage config realm create gosa gosa_ldap
#/opt/privacyidea/bin/pi-manage config policy p_import -f /etc/privacyidea/gosa_policy.cfg
/opt/privacyidea/bin/pi-manage config import -t policy -i /etc/privacyidea/gosa_policy.cfg
/opt/privacyidea/bin/pi-manage config import -i /etc/privacyidea/reverse_proxy_config.json
/opt/privacyidea/bin/pi-manage run -h pi.gosa.example.com -p 443 --cert /srv/certs/pi.gosa.example.com.pem --key /srv/certs/pi.gosa.example.com.pem
