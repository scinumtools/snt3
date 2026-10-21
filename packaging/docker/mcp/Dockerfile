# Reuse the existing REST image's compiled SNT binary and optional report support.
# Build scinumtools3-rest first, or supply --build-arg SNT_REST_IMAGE=your-image.
ARG SNT_REST_IMAGE=scinumtools3-rest
FROM ${SNT_REST_IMAGE}

RUN useradd --create-home --uid 10001 snt-mcp
USER snt-mcp
WORKDIR /home/snt-mcp
EXPOSE 8081

ENTRYPOINT ["/opt/scinumtools3/build/bin/snt", "mcp"]
CMD ["--transport", "streamable-http", "--address", "0.0.0.0"]
