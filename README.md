Set up core components: Django, Postgres, Gunicorn, & Nginx: https://www.digitalocean.com/community/tutorials/how-to-set-up-django-with-postgres-nginx-and-gunicorn-on-ubuntu-18-04
Provide environment variables to the gunicorn service: https://serverfault.com/questions/413397/how-to-set-environment-variable-in-systemd-service

database setup:

(Replace <DB_USER> and <PW> with your values)

```sql
postgres=# CREATE DATABASE django_sensor;
postgres=# CREATE USER <DB_USER> WITH PASSWORD '<PW>';
postgres=# ALTER ROLE <DB_USER> SET client_encoding TO 'utf8';
postgres=# ALTER ROLE <DB_USER> SET default_transaction_isolation TO 'read committed';
postgres=# ALTER ROLE <DB_USER> SET timezone TO 'UTC';
postgres=# GRANT ALL PRIVILEGES ON DATABASE django_sensor TO <DB_USER>;
postgres=# ALTER USER <DB_USER> CREATEDB;
postgres=# ALTER DATABASE django_sensor OWNER TO <DB_USER>;
```
