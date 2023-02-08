# C HTTP Server

This is a simple HTTP server

## Usage

Build docker image

```
docker compose build
```

Start the server

```
docker compose up
```

And then go to localhost:8080/

### On Ubuntu

Compile

```bash
make
```

Start the server

```bash
./vodserver <port>
```

Test the server

```bash
curl localhost:<port>/<filename>
```

Available file list:

```
myfile.css  myfile.html  myfile.js  myfile.py  myfile.jpg  myfile.mp4  myfile.png  myfile.txt
```

## Examples

Start the server

```
./vodserver 8080
```

### 200 OK

```
curl localhost:8080/myfile.txt -v
```

Expected return:

```
*   Trying 127.0.0.1:8080...
* TCP_NODELAY set
* Connected to localhost (127.0.0.1) port 8080 (#0)
> GET /myfile.txt HTTP/1.1
> Host: localhost:8080
> User-Agent: curl/7.68.0
> Accept: */*
>
* Mark bundle as not supporting multiuse
< HTTP/1.1 200 OK
< Last-Modified: Wed, 18 Jan 2023 23:41:28 GMT
< Content-Type: text/plain
< Accept-Ranges: bytes
< Date: Sat, 21 Jan 2023 18:51:39 GMT
< Content-Length: 847
<
* Connection #0 to host localhost left intact
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed non risus. Suspendisse lectus tortor, dignissim sit amet, adipiscing nec, ultricies sed, dolor. Cras elementum ultrices diam. Maecenas ligula massa, varius a, semper congue, euismod non, mi. Proin porttitor, orci nec nonummy molestie, enim est eleifend mi, non fermentum diam nisl sit amet erat. Duis semper. Duis arcu massa, scelerisque vitae, consequat in, pretium a, enim. Pellentesque congue. Ut in risus volutpat libero pharetra tempor. Cras vestibulum bibendum augue. Praesent egestas leo in pede. Praesent blandit odio eu enim. Pellentesque sed dui ut augue blandit sodales. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Aliquam nibh. Mauris ac mauris sed pede pellentesque fermentum. Maecenas adipiscing ante non diam sodales hendrerit.
```

### 206 Partial Content

```
curl -r 5-15 localhost:8080/myfile.txt -v
```

Expexted return:

```
*   Trying 127.0.0.1:8080...
* TCP_NODELAY set
* Connected to localhost (127.0.0.1) port 8080 (#0)
> GET /myfile.txt HTTP/1.1
> Host: localhost:8080
> Range: bytes=5-15
> User-Agent: curl/7.68.0
> Accept: */*
>
* Mark bundle as not supporting multiuse
< HTTP/1.1 206 Partial Content
< Last-Modified: Wed, 18 Jan 2023 23:41:28 GMT
< Content-Type: text/plain
< Accept-Ranges: bytes
< Content-Range: bytes 5-15/847
< Date: Sat, 21 Jan 2023 18:58:02 GMT
< Content-Length: 847
<
* transfer closed with 836 bytes remaining to read
* Closing connection 0
curl: (18) transfer closed with 836 bytes remaining to read
 ipsum dolo%
```

### 404 Not Found

```
curl localhost:8000/labjabujhnoasnhbonb.ogg -v
```

Expected return:

```
*   Trying 127.0.0.1:8080...
* TCP_NODELAY set
* Connected to localhost (127.0.0.1) port 8080 (#0)
> GET /labjabujhnoasnhbonb.ogg HTTP/1.1
> Host: localhost:8080
> User-Agent: curl/7.68.0
> Accept: */*
>
* Mark bundle as not supporting multiuse
< HTTP/1.1 404 Not Found
< Content-Type: text/html
* no chunk, no close, no size. Assume close to signal end
<
<html><body><h1>404 Not Found</h1></body></html>
* Closing connection 0
```

You can also test on a browser to see the image and video showing.
