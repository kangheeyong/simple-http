FROM python:3.9

RUN apt update
RUN apt install gcc -y
RUN apt install netbase -y

RUN pip3 install --upgrade pip
RUN pip3 install fire

RUN mkdir -p /app

WORKDIR /app
