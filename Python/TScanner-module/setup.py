from distutils.core import setup, Extension

tscanner = Extension('tscanner',libraries=['pthread'],sources=['tscanner_mod.c'])

i=setup(name='TScanner.Package',version='1.0',description='Interface for TCP/IP scanning',ext_modules=[tscanner])

# library_dirs=['/usr/lib32','/usr/lib/x86_64-linux-gnu','/usr/libx32'],
