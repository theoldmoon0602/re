FROM alpine

RUN apk --no-cache add gcc libc-dev python3 socat
ADD assets /
RUN chmod a+x server.sh && chmod a+x getandclean.sh
RUN gcc re.c -o re
CMD ["sh", "server.sh", "5432"]
