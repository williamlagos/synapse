# #!/bin/sh
# /opt/btsync/btsync --webui.listen 0.0.0.0:8888

import subprocess
subprocess.call(['/opt/btsync/btsync', '--webui.listen', '0.0.0.0:8888')