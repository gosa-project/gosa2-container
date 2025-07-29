FROM ubuntu:24.04

LABEL maintainer="Benjamin Zapiec <zapiec@gonicus.de>"

ARG HEALTHCHECK_LDAP_USER_DN="cn=manager,dc=example,dc=com"
ARG HEALTHCHECK_LDAP_USER_PW="secret"
ARG HEALTHCHECK_LDAP_BASE="dc=example,dc=com"
ARG HEALTHCHECK_LDAP_URI="ldap://{$HOSTNAME}"

ENV HEALTHCHECK_LDAP_USER_DN=${HEALTHCHECK_LDAP_USER_DN}
ENV HEALTHCHECK_LDAP_USER_PW=${HEALTHCHECK_LDAP_USER_PW}
ENV HEALTHCHECK_LDAP_BASE=${HEALTHCHECK_LDAP_BASE}
ENV HEALTHCHECK_LDAP_URI=${HEALTHCHECK_LDAP_URI}

RUN apt-get update && DEBIAN_FRONTEND=noninteractive \
  apt-get -y install \
    xz-utils \
    curl \
    ldap-utils \
    slapd \
    ssl-cert \
    ca-certificates \
    sasl2-bin \
  && apt-get clean \
  && rm -rf /var/lib/apt/lists /var/cache/apt/archives

RUN rm -rf /var/lib/ldap && mkdir /var/lib/ldap
RUN mkdir -p /etc/opt/init/schema
COPY assets/schema /etc/opt/init/schema/
COPY assets/config.ldif /etc/opt/init/
COPY assets/data.ldif /etc/opt/init/

# RUN mkdir /certs
# COPY certs/ldaps.cert /certs/
# COPY certs/ldaps.csr /certs/
# COPY certs/ldaps.key /certs/

# recreate initial directores
RUN rm -rf /etc/ldap /var/lib/ldap && mkdir -pv /etc/ldap/sasl2 /etc/ldap/slapd.d /var/lib/ldap 
RUN mkdir -pv /var/lib/ldap/gosadb
RUN mkdir -pv /var/lib/ldap/accesslog

# configure saslauthd
#COPY assets/saslauthd /etc/default/saslauthd
RUN echo "mech_list: plain" >> /etc/ldap/sasl2/slapd.conf \
  && echo "pwcheck_method: saslauthd" >> /etc/ldap/sasl2/slapd.conf \
  && echo "saslauthd_path: /run/saslauthd/mux" >> /etc/ldap/sasl2/slapd.conf

COPY --chmod=0755 docker-entrypoint.sh /usr/local/bin
COPY --chmod=0755 docker-healthcheck.sh /usr/local/bin

EXPOSE 389 636
VOLUME ["/var/lib/ldap"]
ENTRYPOINT ["docker-entrypoint.sh"]
HEALTHCHECK --interval=10s --timeout=10s --start-period=5s --retries=3 CMD [ docker-healthcheck.sh ]
