FROM ubuntu:24.04

ARG PHP_VERSION="8.2"
ENV GOSA_PHP_VERSION=${PHP_VERSION}
ENV DEBIAN_FRONTEND=noninteractive

LABEL maintainer="Benjamin Zapiec <zapiec@gonicus.de>"

RUN apt-get update && \
  apt-get -y install \
    curl \
    ssl-cert \
    ca-certificates \
    nginx \
    software-properties-common \
    git \
    libnet-ldap-perl \
    liblog-log4perl-perl \
    libmodern-perl-perl \
    libconfig-file-perl \
    libdbd-mysql-perl \
    libexpect-perl \
    krb5-user \
    krb5-config \
    debconf-utils \
    vim-nox \
    s-nail \
    gettext \
    imagemagick \
    sudo \
    joe \
    python3-pip
    
RUN add-apt-repository -y ppa:ondrej/php
RUN apt-get -y update
RUN apt-get -y install \
      ldap-utils \
      php${GOSA_PHP_VERSION}-cli \
      php${GOSA_PHP_VERSION}-curl \
      php${GOSA_PHP_VERSION}-fpm \
      php${GOSA_PHP_VERSION}-gd \
      php${GOSA_PHP_VERSION}-imagick \
      php${GOSA_PHP_VERSION}-imap \
      php${GOSA_PHP_VERSION}-intl \
      php${GOSA_PHP_VERSION}-ldap \
      php${GOSA_PHP_VERSION}-mbstring \
      php${GOSA_PHP_VERSION}-mysql \
      php${GOSA_PHP_VERSION}-readline \
      php${GOSA_PHP_VERSION}-xml \
      composer \
      php-json \
      php-fpdf \
      locales

RUN update-alternatives --set php /usr/bin/php${GOSA_PHP_VERSION}

COPY assets/php-fpm.conf /etc/php/${GOSA_PHP_VERSION}/fpm
COPY assets/php.ini /etc/php/${GOSA_PHP_VERSION}/fpm
COPY assets/sudoers_www-data /etc/sudoers.d/www-data
COPY assets/init-gosa.php /
COPY assets/class_ldap.inc /

RUN mkdir -p /srv/gosa/gosa
COPY assets/gosa2 "/srv/gosa/gosa/gosa-core"

RUN apt-get clean \
  && rm -rf /var/lib/apt/lists /var/cache/apt/archives

RUN mkdir /etc/gosa
COPY assets/gosa.conf /etc/gosa
COPY assets/nginx_gosa /etc/nginx/sites-available/gosa
RUN ln -s ../sites-available/gosa /etc/nginx/sites-enabled/
RUN rm /etc/nginx/sites-enabled/default
RUN locale-gen de

COPY --chmod=0755 docker-entrypoint.sh /

EXPOSE 80
ENTRYPOINT [ "/docker-entrypoint.sh" ]
VOLUME ["/srv/gosa"]
