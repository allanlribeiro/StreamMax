import sys
from pathlib import Path
text=Path(sys.argv[1]).read_text(errors='replace')[-5000:]
print('::error::'+text.replace('%','%25').replace('\r','%0D').replace('\n','%0A'))
