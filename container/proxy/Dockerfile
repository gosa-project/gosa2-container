FROM ubuntu:26.04

LABEL maintainer="Jan Wenzel <wenzel@gonicus.de>"

RUN apt-get update && DEBIAN_FRONTEND=noninteractive \
    apt-get -y install \
    dante-server \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists /var/cache/apt/archives
COPY assets/danted.conf /etc/danted.conf
# COPY assets/nginx_reverse-proxy /etc/nginx/conf.d/reverse-proxy.conf
# RUN rm /etc/nginx/conf.d/default.conf
COPY --chmod=0755 docker-entrypoint.sh /docker-entrypoint.sh

EXPOSE 1080
ENTRYPOINT [ "/docker-entrypoint.sh" ]
