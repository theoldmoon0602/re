# build

```shell
$ docker-compose build
```

# run

```shell
$ docker-compose up -d
```

# stop

```shell
$ docker-compose down
```

# attack flags

 flag format is `flag-[0-9A-Za-z]{32}`

- `flag-455472b522fd0a1e27aaa91c4033db65`
- `flag-005a1c58a8c4ac2203cc01d6d4ed438f`
- `flag-c782a0a5d3c71ac7f3d071272f58812d`

# defense token

 token format is `token-[0-9A-Za-z]{32}`

## check token

 **this script delete all tokens**

```python
from . import def_re

print(def_re.tokens())
```
