kill still kills even though child process already exited

http parser doesnt handle whitespace correctly when parsing headers. I think.

Should implement signal to gracefully exit server with closing open fds.

Two request in the same buffer. How handle?

when multiple servers listen on the same address(port), bind would fail instead of ignore (it should probably work and use host to match which server it routes to)

Does it matter all log info are printing to cout and not cerr?
Is using printf alright? (Seems like it... variadic template would be preferred way of doing that but it's a c++11 feature)


Parser:
Duplicate handled only for location (uri).
For error page the first will apply (insert call will fail if key exists).

HTTP request can contain duplicate headers? How is this supposed to be handled?
```GET /example HTTP/1.1
Host: example.com
Cookie: session_id=12345
Cookie: user_id=67890```

Should cookie be saved in the server& (different ones will have different cookies)?
