# Dockerfile
FROM alpine:latest

# Instalar herramientas necesarias de red y build
RUN apk add --no-cache \
    bash \
    iproute2 \
    can-utils \
    build-base \
    cmake \
    git \
    linux-headers \
    openssh

RUN apk add --no-cache \
    openssh \
    rsync

# Crear punto de trabajo
WORKDIR /opt/host

# Por defecto, shell interactivo
# ENTRYPOINT ["/bin/sh"]
