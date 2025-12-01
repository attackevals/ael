# to run on docker host, to install certificate from authentik to guac as trusted

# step 1.

# get cert from authentik
# navigate to authentik, settings -> System -> Certificates
# generate new cert
# use domain name for cn and san (sso.kingslanding.net)
# download certificate
# copy certificate to docker host for guac
# copy cert to guac
# the record share is already mounted on guacamole
# docker cp /tmp/sso.kingslanding.net.cert.pem /opt/guacamole/record/
# trust certificate
# docker exec -u root --name guacamole_compose keytool -keystore /opt/java/openjdk/jre/lib/security/cacerts -storepass changeit -noprompt -trustcacerts -import -alias my-ca -file /record/sso.kingslanding.net.cert.pem
# restart container
# docker compose restart
