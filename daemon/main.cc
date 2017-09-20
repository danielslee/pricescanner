#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <string>
#include <iostream>
#include <ApplicationServices/ApplicationServices.h>

#include "mongoose.h"
#include "price.h"

std::string jsonPrices() {
  char* outText;
  std::stringstream sstream;

  tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
  if (api->Init(NULL, "eng")) {
      fprintf(stderr, "Could not initialize tesseract.\n");
      exit(1);
  }

  api->SetPageSegMode(tesseract::PSM_SPARSE_TEXT);

  // take a screenshot (OS X)
  CGImageRef imageRef = CGDisplayCreateImage(CGMainDisplayID());
  CFDataRef dataRef = CGDataProviderCopyData(CGImageGetDataProvider(imageRef));

  size_t width, height;
  width = CGImageGetWidth(imageRef);
  height = CGImageGetHeight(imageRef);
  size_t bpp = CGImageGetBitsPerPixel(imageRef);

  Pix *image = pixCreate(width, height, bpp);
  memcpy(image->data, CFDataGetBytePtr(dataRef), width * height * (bpp/8));
  image->wpl = CGImageGetBytesPerRow(imageRef)/4;
  image->colormap = NULL;

  pixEndianByteSwap(image);
  api->SetImage(image);

  // extract the text and prices
  outText = api->GetUTF8Text();

  std::vector<Price> prices = Price::PricesInText(outText);
  sstream << "{\"prices\":[";
  int i = 0;
  for (Price& p : prices) {
    sstream << "{\"amount\":" << p.amount() << ",\"currency\":\""
            << p.currencyCode() << "\"}";
    if (++i < prices.size()) sstream << ",";
  }
  sstream << "]}";

  delete [] outText;
  api->End();
  CFRelease(dataRef);
  CGImageRelease(imageRef);
  pixDestroy(&image);
  return sstream.str();
}

static void httpHandler(struct mg_connection *c, int ev, void *p) {
  if (ev == MG_EV_HTTP_REQUEST) {
    struct http_message *hm = (struct http_message *) p;
    std::string json = jsonPrices();
    mg_send_head(c, 200, json.length(), "Content-Type: application/json\n"\
                                        "Access-Control-Allow-Origin: *");
    mg_printf(c, "%s", json.c_str());
  }
}

int main(int argc, char** argv) {
  struct mg_mgr mgr;
  struct mg_connection *c;
  mg_mgr_init(&mgr, NULL);

  c = mg_bind(&mgr, "1234", httpHandler);
  mg_set_protocol_http_websocket(c);

  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }

  mg_mgr_free(&mgr);
  return 0;
}
