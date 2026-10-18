"""Extract primary archive names from the pinned Petari factory; no actor behavior."""
import pathlib, re, sys
source = pathlib.Path(sys.argv[1]).read_text(encoding="utf-8")
start = source.index("const NameObjFactory::Name2CreateFunc cCreateTable[]")
table = source[start:source.index("\n    };", start)]
blocks = re.findall(r'\{\s*"([^"\n]+)"\s*,\s*([^{}]+?),\s*("([^"\n]+)"|nullptr)\s*,?\s*\}', table, re.S)
expected = len(re.findall(r'\{\s*"', table))
assert len(blocks) == expected and expected > 500, (len(blocks), expected)
pairs = [(name, archive) for name, creator, value, archive in blocks if archive]
lines = ['#pragma once', '// Generated from pinned Petari NameObjFactory cCreateTable primary archives.',
         'inline const char* factoryPrimaryArchive(const std::string& name){',
         '    static const std::pair<const char*,const char*> entries[]={']
lines += ['        {"%s","%s"},' % pair for pair in pairs]
lines += ['    };', '    auto equal=[](const std::string& a,const char* b){std::string value(b);if(a.size()!=value.size())return false;for(size_t i=0;i<a.size();++i){auto lower=[](char c){return c>=\'A\'&&c<=\'Z\'?char(c-\'A\'+\'a\'):c;};if(lower(a[i])!=lower(value[i]))return false;}return true;};',
          '    for(auto entry:entries)if(equal(name,entry.first))return entry.second;return nullptr;', '}']
pathlib.Path(sys.argv[2]).write_text('\n'.join(lines)+'\n', encoding='utf-8')
print(f'Extracted {len(pairs)} primary archives from {expected} factory entries')
