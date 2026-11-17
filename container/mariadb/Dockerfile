FROM mariadb:11.8

LABEL maintainer="Benjamin Zapiec <zapiec@gonicus.de>"

COPY assets/gosa_log.sql /docker-entrypoint-initdb.d
#COPY assets/metadb.sql /docker-entrypoint-initdb.d
COPY assets/pi.sql /docker-entrypoint-initdb.d

HEALTHCHECK CMD ["healthcheck.sh", "--su=mysql", "--connect", "--innodb_initialized"]
