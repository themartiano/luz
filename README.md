# miniRT

### Command-line arguments:

* -s: Custom sample ammout
* -mb: Custom maximum light bounces
* -th: Custom rendering thread number
* --save: Saves render to .bmp file
* --no-window: Renders without showing the result in a window

### Compilation flags

* DEBUG: Adds the `-g` flag
* NOFLAGS: Removes `-wall`, `-wextra` and `-werror` flags
* SANITIZER: Adds the `-g` and `-fsanitize=address` flags

##### Usage example: make NOFLAGS=1
