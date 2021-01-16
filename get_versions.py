import io
import json
import os

import urllib.request
import gzip
import ssl

from src.client_config import ClientConfig

config = ClientConfig()

config_file = os.path.join(".pyupdater", "config.pyu")


def get_dict_from_url():
    name = config.UPDATE_URLS[0] + "versions.gz"
    print("Retrieving ", name)
    ctx = ssl.create_default_context()
    try:
        response = urllib.request.urlopen(name, context=ctx)
    except OSError as e:
        print("Remote Version not found, ", e)
        return {}
    else:
        g = gzip.open(response)
        return json.load(g)


with open(config_file, "rb") as f:
    config_data = json.load(f)
remote_data = get_dict_from_url()
if "updates" in remote_data:
    config_data["version_meta"] = get_dict_from_url()
print(json.dumps(config_data, indent=4, sort_keys=True))
with open(config_file, "w") as outfile:
    json.dump(config_data, outfile, indent=4, sort_keys=True)
