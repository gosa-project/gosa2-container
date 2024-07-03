#!/bin/bash

# cp /srv/certs/root_ca.pem /usr/local/share/ca-certificates/root_ca.crt
# update-ca-certificates

# echo "Generating x509 wildcard certificate (if not exists)"
# certauth -d /srv/certs/ --hostname "example.com" -w /srv/certs/root_ca_cert.pem || /bin/true

rm -rf /var/spool/gosa
mkdir /var/spool/gosa
chown www-data: /var/spool/gosa

# echo "Generating gosa class_location.inc"
# pushd /srv/gosa/gosa/gosa-core
# ./update-gosa rescan-classes
# popd

pushd /srv/gosa/gosa/gosa-core
echo "Generating gosa i18n"
./update-gosa rescan-i18n
popd

echo "Starting postfix"
service postfix start

echo "Starting php${GOSA_PHP_VERSION}-fpm"
service php${GOSA_PHP_VERSION}-fpm start

echo 'TLS_REQCERT never' >> /etc/ldap/ldap.conf
sleep 5 # ugly "fix" to wait a little longer for the ldap to become ready
php /init-gosa.php "$LDAP_LIST" "$ROOT_DN" "$ROOT_PW" $USE_TLS

composer update /srv/gosa/gosa/gosa-core/

echo "Starting nginx"
nginx -g 'daemon off;error_log /dev/stdout info;'
