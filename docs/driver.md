## Drivers

To make a driver make a driver_t object like this


static driver_t driver = {
  .probe = probe,
  .attach = attach,
  .detach = detach
}


and then call 
RegisterDriver(&driver);
