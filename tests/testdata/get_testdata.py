import os
import sys
import urllib.request

nargs = len(sys.argv)

xml = "https://arts.mi.uni-hamburg.de/svn/rt/arts-xml-data/branches/arts-xml-data-2.6/"
cat = "https://arts.mi.uni-hamburg.de/svn/rt/arts-cat-data/branches/arts-cat-data-2.6/"

if nargs > 3:
    try:
        path = os.getcwd()
        os.chdir(sys.argv[1])
        if sys.argv[2] == "xml":
            baseurl = xml
            basedir = "arts-xml-data"
        elif sys.argv[2] == "cat":
            baseurl = cat
            basedir = "arts-cat-data"
        else:
            baseurl = ""
            basedir = "custom-data"
        
        for file in sys.argv[3:]:
            f = f"{basedir}/{file}"
            if not os.path.exists(f):
                os.makedirs(os.path.split(f)[0], exist_ok=True)
                print(f"Downloading {f}")
                urllib.request.urlretrieve(f"{baseurl}{file}", f)
            else:
                print(f"Skipping {file}, exists at {os.path.abspath(f)}")
    finally:
        os.chdir(path)

