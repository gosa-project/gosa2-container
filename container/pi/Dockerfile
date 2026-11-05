FROM ubuntu:22.04

LABEL maintainer="Jan Wenzel <wenzel@gonicus.de>"

RUN apt-get update && DEBIAN_FRONTEND=noninteractive \
  apt-get -y install \
    curl \
    nginx \
    software-properties-common \
    git \
    gnupg \
    python3 \
    python3-pip \
    python3-venv \
    nginx

RUN apt-get clean \
  && rm -rf /var/lib/apt/lists /var/cache/apt/archives

RUN python3 -mvenv /opt/privacyidea
RUN /opt/privacyidea/bin/pip install -U pip
RUN /opt/privacyidea/bin/pip install setuptools
RUN /opt/privacyidea/bin/pip install wheel
RUN /opt/privacyidea/bin/pip install pymysql-sa
RUN /opt/privacyidea/bin/pip install PyMySQL
RUN /opt/privacyidea/bin/pip install -r https://raw.githubusercontent.com/privacyidea/privacyidea/v3.10/requirements.txt
RUN /opt/privacyidea/bin/pip install privacyidea==3.10.*

COPY assets/nginx_pi.conf /etc/nginx/sites-available/pi.gosa.conf
RUN ln -s ../sites-available/pi.gosa.conf /etc/nginx/sites-enabled/pi.gosa.conf
RUN rm /etc/nginx/sites-enabled/default

RUN mkdir /etc/privacyidea
COPY assets/pi.cfg /etc/privacyidea
COPY assets/gosa_ldapresolver.conf /etc/privacyidea
COPY assets/gosa_policy.cfg /etc/privacyidea
COPY assets/reverse_proxy_config.json /etc/privacyidea

COPY docker-entrypoint.sh /
ENTRYPOINT [ "/docker-entrypoint.sh" ]
EXPOSE 80
EXPOSE 443

# RUN curl https://lancelot.netknights.it/NetKnights-Release.asc | gpg --dearmor > /etc/apt/trusted.gpg.d/privacy-idea.gpg
# RUN add-apt-repository http://lancelot.netknights.it/community/jammy/stable
# RUN apt-get -y install privacyidea-nginx
