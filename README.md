# Price scanner
This is a simple demo which takes a screenshot, finds all prices using a
combination of OCR (Tesseract) and heurestics and then displays them along with
an approximate value in USD for non-USD currencies.

## Architecture
This demo consists of a C++ daemon responsible for taking the screenshots
and extracting/normalizing the prices and an Angular app displaying everything
and providing conversions to USD where applicable.

## Build
You can build the daemon on a Mac as follows: (before building make sure you
have intstalled all of the linked libraries)

```
clang++ -o daemon main.cc price.cc mongoose.c -llept -ltesseract -lpcre -framework ApplicationServices
```

The interface can be run using ng serve.

## Caveats
- Only USD, EUR and GBP are hardcoded into the deamon. It would be easy to load
  more currencies along with their symbols ($€£) from a file.
- The screenshot taking code only works on macOS. It should be easy enough to
  implement this functionality for other systems as well.
- The OCR should tweaked to achieve higher precision.

## License
You can use all of my code under the terms and conditions of the 2-clause
BSD License. The dameon contains mongoose http server code licensed under
GPLv2, therefore the whole daemon must be licensed under GPLv2. You can use it
under BSD2 if you remove/replace the above mentioned GPL code.
