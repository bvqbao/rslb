A Really Simple Load Balancer
---------------------

Require *luajit* installed on your machine (e.g. sudo apt install libluajit-5.1-dev).

### Run RSLB:

```bash
cd rslb
make clean && make
./rslb ../config.lua
```

### Run backends:

Open one terminal for each backend/server.

#### Terminal 1:

```bash
cd backends
./run.sh 127.0.0.1 8081
```

#### Terminal 2:

```bash
cd backends
./run.sh 127.0.0.1 8082
```
...

Make sure that the backend addresses are listed in the RSLB configuration file.

To test the load balancer:

 * HTML documents: http://127.0.0.1:9000/htdocs/hello.html
 * Scripts (python): http://127.0.0.1:9000/cgi-bin/test.py

Check the RSLB log output to see that the load balancer balances the loads in a round-robin way.
