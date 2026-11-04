#!/bin/bash

cp /srv/certs/root_ca.pem /usr/local/share/ca-certificates/root_ca.crt
update-ca-certificates

# service nginx start

/opt/privacyidea/bin/pi-manage create_tables
/opt/privacyidea/bin/pi-manage create_enckey
/opt/privacyidea/bin/pi-manage create_audit_keys
/opt/privacyidea/bin/pi-manage admin add -p gosa-admin-secret gosa-admin
/opt/privacyidea/bin/pi-manage resolver create gosa_ldap ldapresolver /etc/privacyidea/gosa_ldapresolver.conf
/opt/privacyidea/bin/pi-manage realm create gosa gosa_ldap
/opt/privacyidea/bin/pi-manage policy p_import -f /etc/privacyidea/gosa_policy.cfg
/opt/privacyidea/bin/pi-manage config importer -i /etc/privacyidea/reverse_proxy_config.json
/opt/privacyidea/bin/pi-manage runserver -h pi.gosa.example.com -p 443 --ssl-crt /srv/certs/pi.gosa.example.com.pem --ssl-key /srv/certs/pi.gosa.example.com.pem
