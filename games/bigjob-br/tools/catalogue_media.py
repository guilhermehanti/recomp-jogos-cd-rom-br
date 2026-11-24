"""Reproducible static media catalogue; does not execute the game."""
import json
import re
import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main():
    mapping = json.loads((ROOT / 'analysis/mapa-jogo-v3.json').read_text(encoding='utf-8-sig'))
    flow = json.loads((ROOT / 'analysis/mms-control-flow.json').read_text(encoding='utf8'))
    binary = (ROOT / 'build/BIGJOBNG.EXE').read_bytes()
    ds = next(s for s in mapping['executables']['BIGJOBNG.EXE']['segments'] if s['id'] == 49)
    awards = list(struct.unpack_from('<36H', binary, ds['offset'] + 0x204))
    expected = [24,36,44,17,49,65535,25,48,46,56,15,7,55,27,65535,57,0,11,21,5,29,31,65535,41,4,34,65535,51,43,65535,40,32,23,54,65535,13]
    assert awards == expected
    movies = [dict(id=i, **movie, selectors=[]) for i, movie in enumerate(mapping['movies'])]
    assert len(movies) == 58
    for theme, table, ids in [(0,'44:00A6',[14,6,52,47,38,39,1]),(1,'44:01F6',[10,28,50,53,37,33,30]),('other','49:09A8',[3,12,16,42,26,45,35])]:
        for column, mid in enumerate(ids):
            movies[mid]['selectors'].append(dict(group='cabin',scene=2,theme=theme,column=column,table=table))
    for theme, mid in [(0,20),(1,18),('other',8)]:
        movies[mid]['selectors'].append(dict(group='CP',scene=4,theme=theme,evidence='9:19F5-1A19'))
    for button, mid in enumerate([2,9,19,22]):
        movies[mid]['selectors'].append(dict(group='jukebox',scene=12,button=button,evidence='26:34AC/355A/3608/36BD ->4:22A9 ->4:239D'))
    for progress, mid in enumerate(awards):
        if mid != 65535:
            movies[mid]['selectors'].append(dict(group='award',progress_id=progress,table=f'49:{0x204+2*progress:04X}',evidence='3:12DD ->3:098A ->4:1929'))
    assert all(len(m['selectors']) == 1 for m in movies)
    activities = []
    for scene, progress, call in [(7,31,'6:1AF4'),(13,12,'25:33B9'),(15,32,'21:0116'),(16,19,'18:071B'),(17,18,'19:0AC6'),(18,15,'27:252F'),(19,4,'29:0585'),(20,6,'16:08BE'),(21,23,'17:002D')]:
        activities.append(dict(scene=scene,progress_id=progress,award_movie=awards[progress],base_call=call))
    translation = next(s for s in mapping['executables']['BIGJOBNG.EXE']['segments'] if s['id'] == 43)
    cabin_progress = []
    for index in range(21):
        movie, progress, reserved = struct.unpack_from('<IHH', binary, translation['offset'] + index * 8)
        cabin_progress.append(dict(movie_id=movie, progress_id=progress, award_movie=awards[progress],
                                   reserved=reserved, table=f'43:{index * 8:04X}',
                                   evidence='12:2EA6 ->12:2E34 ->6:026C'))
    all_progress = [row['progress_id'] for row in activities + cabin_progress]
    valid_progress = [index for index, movie in enumerate(awards) if movie != 65535]
    assert sorted(all_progress) == valid_progress
    assert len(all_progress) == len(set(all_progress)) == 30
    sounds = []
    for bank, name in enumerate(['BIGJOB.SND','BIGVOX.SND']):
        sounds.extend(dict(packed_id=(bank << 14) | s['index'],bank=bank,bank_file=name,**s) for s in mapping['banks'][name])
    triggers = []
    for script in flow:
        label = None
        for record in script['records']:
            if record['opcode'] == 12:
                label = record['label']
            if 'sound_id' in record:
                triggers.append(dict(root_id=script['root_id'],block=script['block'],preceding_label=label,**record))
    # Keep literal native argument windows: values may be dynamic or assembled
    # from multiple pushes, so these are evidence rather than guessed sound IDs.
    native = []
    for path in sorted((ROOT / 'analysis/disassembly').glob('seg*.asm')):
        lines = path.read_text(encoding='utf8').splitlines()
        for i,line in enumerate(lines):
            if re.search(r'=36:(28F7|2937)\b', line):
                native.append(dict(segment=int(path.stem[3:]),site=line.strip(),argument_window=lines[max(0,i-14):i],status='literal argument window; dynamic arguments not resolved'))
    result = dict(method='static only',movie_count=58,movies=movies,award_table=awards,
                  activities_with_proven_progress_id=activities,
                  cabin_completion_progress=cabin_progress,
                  valid_award_contexts_verified=30,
                  sounds=sounds,mms_audio_triggers=triggers,native_audio_calls=native,
                  limitations=['A script root is not proof of UI reachability.',
                               'Shared native segments are not assigned wholesale to one scene.',
                               'Dynamic native audio arguments require owner and state tracing.'])
    output = ROOT / 'analysis/media-catalogue.json'
    output.write_text(json.dumps(result,ensure_ascii=False,indent=2),encoding='utf8')
    print(f'58 movie selectors; {len(sounds)} sounds; {len(triggers)} MMS triggers; {len(native)} native audio calls; {len(activities)} proven scene/progress pairs')


if __name__ == '__main__':
    main()
