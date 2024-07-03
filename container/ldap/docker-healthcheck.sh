#!/bin/bash

set -eu

ldapsearch \
  -x \
  -D ${HEALTHCHECK_LDAP_USER_DN} \
  -w ${HEALTHCHECK_LDAP_USER_PASSWORD} \
  -H ${HEALTHCHECK_LDAP_URI} \
  -b ${HEALTHCHECK_LDAP_BASE} \
  -s base 