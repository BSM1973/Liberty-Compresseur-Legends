from pathlib import Path
import sys, re
bad=[]
for p in list(Path("src").glob("*.h"))+list(Path("src").glob("*.cpp"))+list(Path("docs").glob("*.md")):
    s=p.read_text(encoding="utf-8",errors="strict")
    for i,line in enumerate(s.splitlines(),1):
        if any(x in line for x in ("\ufffd","\u00c3\u00a9","\u00c3\u00a8","\u00c3\u00a0","\u00e2\u0080","\u00c2\u00a0")) or any(0x80<=ord(c)<=0x9f for c in line):
            bad.append(f"{p}:{i}: suspicious encoding")
print("\n".join(bad) if bad else "UTF-8 source scan passed")
sys.exit(bool(bad))
