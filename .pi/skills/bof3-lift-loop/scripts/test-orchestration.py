#!/usr/bin/env python3
"""Focused orchestration and parent-managed worktree self-check."""

import json
from pathlib import Path
import subprocess
import tempfile
import uuid

ROOT = Path(__file__).resolve().parents[4]
SCRIPTS = ROOT / ".pi/skills/bof3-lift-loop/scripts"
SELECTOR = "emi/etc/shop/00@0x801DDFB0"


def run(*args: str, check: bool = True) -> subprocess.CompletedProcess[str]:
    return subprocess.run(args, cwd=ROOT, text=True, capture_output=True, check=check)


def rejected(*args: str) -> None:
    assert run(*args, check=False).returncode != 0


def main() -> int:
    run_id = uuid.uuid4().hex
    key = f"orchestration-self-check-{run_id}"
    with tempfile.TemporaryDirectory() as directory:
        output = Path(directory) / "lane.js"
        common = ("--selector", SELECTOR, "--run-key", key, "--output", str(output))
        run("python3", str(SCRIPTS / "render-workflow.py"), "render", *common)
        assert json.loads(
            run(
                "python3", str(SCRIPTS / "render-workflow.py"), "verify", *common
            ).stdout
        )["verified"]
        syntax = Path(directory) / "syntax.js"
        rendered = output.read_text()
        syntax.write_text("async function lane(){\n" + rendered + "\n}\n")
        run("node", "--check", str(syntax))
        behavior = Path(directory) / "behavior.js"
        behavior.write_text(
            "const saved = {}, calls = []; let cleanupTask = '';\n"
            "const state = {get: async k => saved[k], set: async (k,v) => {saved[k]=v;}};\n"
            "const gate = metric => ({ok:true,results:[{acceptance:{verifyRuns:[{stdout:JSON.stringify({accepted:true,improved:true,checkpoint:'attempt-1-deadbeefdeadbeef',current:{metric,checkpoint:'attempt-1-deadbeefdeadbeef'}})}]}}]});\n"
            "const measured = {ok:true,output:JSON.stringify({status:'exact',match_percent:100,files_changed:[],prepared_rows:['emi/etc/shop/00@function:func_801DDFB0']})};\n"
            "const runs = {run: async (k,o) => {calls.push(k); if(k==='cleanup') cleanupTask=o.task; return k === 'baseline' || k === 'final-measure' ? measured : "
            "k === 'checkpoint-baseline' ? gate({match_percent:100,exact:true}) : "
            "k === 'checkpoint-pre-cleanup' ? gate({match_percent:100,exact:true}) : "
            "k === 'load-durable-ledger' ? {ok:true,results:[{acceptance:{verifyRuns:[{stdout:JSON.stringify({entries:[]})}]}}]} : "
            "k.startsWith('record-ledger-') ? {ok:true} : "
            "k === 'integrate' ? {ok:true,results:[{acceptance:{verifyRuns:[{stdout:JSON.stringify({integrated:true,commit:'test'})}]}}]} : "
            "{ok:true,output:JSON.stringify({verdict:'pass',prepared_rows:['emi/etc/shop/00@function:func_801DDFB0']})};}};\n"
            "async function lane(){\n" + rendered + "\n}\n"
            "lane().then(v => console.log(JSON.stringify({result:v,state:saved,calls,cleanupTask})));\n"
        )
        result = json.loads(run("node", str(behavior)).stdout)
        assert result["result"]["status"] == "integrated"
        assert result["result"]["attempt"] == 0
        assert result["result"]["bestScore"] == 100
        assert result["state"]["lane"]["status"] == "integrated"
        assert "cleanup" in result["calls"]
        assert result["cleanupTask"].endswith(
            "exact emi/etc/shop/00@function:func_801DDFB0"
        )
        assert result["result"]["cleanup"] == {
            "verdict": "pass",
            "prepared_rows": ["emi/etc/shop/00@function:func_801DDFB0"],
        }
        assert 'const cleanupRequest = ["retained-lift"' in rendered
        assert "task: cleanupRequest" in rendered
        assert 'cleanupRequest.startsWith("repair ")' in rendered
        assert "consolidation-review" in result["calls"]
        assert "integrate" in result["calls"]
        assert result["calls"].count("checkpoint-baseline") == 1
        assert result["calls"].count("checkpoint-pre-cleanup") == 1
        assert "--paths-only --target-scope" in rendered
        assert "const restoreCheckpoint = checkpoint =>" in rendered
        assert "restoreCheckpoint(lane.cleanupCheckpoint)" in rendered
        assert (
            "recoveryCheckpoint = cleanupPending ? lane.bestCheckpoint : cleanupPhase ? lane.cleanupCheckpoint : lane.bestCheckpoint"
            in rendered
        )
        assert 'lane.phase === "best-promoting"' in rendered
        assert "gate: inspectBest" in rendered
        assert "gate: restoreCheckpoint(recoveryCheckpoint)" in rendered
        assert 'lane.phase = "cleanup-pending"' in rendered
        assert rendered.index("lane.cleanupCheckpoint = cleanupLeaf") < rendered.index(
            'lane.phase = "cleanup";'
        )
        for invalid_rows in (
            "['unknown:name']",
            "['function:func_801DDFB0','function:func_801DDFB0']",
            "['function:z_name','data:a_name']",
            "['function:path/name']",
            "['exe/logo@function:func_801DDFB0']",
        ):
            invalid = Path(directory) / (
                "invalid-" + str(abs(hash(invalid_rows))) + ".js"
            )
            invalid.write_text(
                behavior.read_text().replace(
                    "['emi/etc/shop/00@function:func_801DDFB0']", invalid_rows
                )
            )
            assert run("node", str(invalid), check=False).returncode != 0
        assert not any(call.startswith("restore-") for call in result["calls"])
        assert "acceptance: false" in rendered
        partial = Path(directory) / "partial.js"
        partial.write_text(
            behavior.read_text()
            .replace(
                "const saved = {}, calls = [];",
                "const saved = {lane:{selector:'emi/etc/shop/00@0x801DDFB0',attempt:20,bestScore:60,status:'attempt-limit',phase:'ready',queue:[],seen:[],ledger:[{attempt:0,score:50}],rung:4,stalledQueues:0,historyLoaded:true}}, calls = [];",
            )
            .replace(
                "k === 'baseline' || k === 'final-measure' ? measured",
                "k === 'baseline' ? measured : k === 'final-measure' ? {ok:true,output:JSON.stringify({status:'partial',match_percent:60,files_changed:[],prepared_rows:['emi/etc/shop/00@function:func_801DDFB0']})}",
            )
        )
        partial_result = json.loads(run("node", str(partial)).stdout)
        assert partial_result["result"]["status"] == "integrated"
        assert partial_result["cleanupTask"].endswith(
            "improved-partial emi/etc/shop/00@function:func_801DDFB0"
        )
        cleanup_failure = Path(directory) / "cleanup-failure.js"
        cleanup_failure.write_text(
            partial.read_text().replace(
                "k === 'integrate' ?",
                "k === 'cleanup' ? {ok:false,output:'{}'} : k === 'restore-cleanup-failure' ? {ok:true} : k === 'integrate' ?",
            )
        )
        cleanup_failed = json.loads(run("node", str(cleanup_failure)).stdout)
        assert cleanup_failed["result"]["status"] == "cleanup-blocked-restored"
        assert cleanup_failed["result"]["bestScore"] == 60
        assert "restore-cleanup-failure" in cleanup_failed["calls"]
        assert cleanup_failed["calls"].index("checkpoint-pre-cleanup") < cleanup_failed[
            "calls"
        ].index("cleanup")
        consolidation_failure = Path(directory) / "consolidation-failure.js"
        consolidation_failure.write_text(
            partial.read_text().replace(
                "k === 'integrate' ?",
                "k === 'consolidation-review' ? {ok:false,output:JSON.stringify({verdict:'block'})} : k === 'restore-consolidation-failure' ? {ok:true} : k === 'integrate' ?",
            )
        )
        consolidation_failed = json.loads(
            run("node", str(consolidation_failure)).stdout
        )
        assert (
            consolidation_failed["result"]["status"] == "consolidation-blocked-restored"
        )
        assert consolidation_failed["result"]["bestScore"] == 60
        assert "restore-consolidation-failure" in consolidation_failed["calls"]
        assert consolidation_failed["calls"].index(
            "checkpoint-pre-cleanup"
        ) < consolidation_failed["calls"].index("cleanup")
        crash = Path(directory) / "crash-boundaries.js"
        crash.write_text(
            "const scenarios = ["
            "{name:'final-measure',phase:'final-measure',cleanup:null,expect:'attempt-1-deadbeefdeadbeef'},"
            "{name:'cleanup-pending',phase:'cleanup-pending',cleanup:null,expect:'attempt-1-deadbeefdeadbeef'},"
            "{name:'cleanup',phase:'cleanup',cleanup:'attempt-21-0123456789abcdef',expect:'attempt-21-0123456789abcdef'},"
            "{name:'cleanup-restore',phase:'cleanup-restore',cleanup:'attempt-21-0123456789abcdef',expect:'attempt-21-0123456789abcdef'},"
            "{name:'consolidation-restore',phase:'consolidation-restore',cleanup:'attempt-21-0123456789abcdef',expect:'attempt-21-0123456789abcdef'},"
            "{name:'integrate',phase:'integrate',cleanup:null,expect:'attempt-1-deadbeefdeadbeef'}"
            "];\n"
            "const base = {selector:'emi/etc/shop/00@0x801DDFB0',attempt:20,bestScore:100,status:'exact',queue:[],seen:[],ledger:[{attempt:0,score:100}],rung:0,stalledQueues:0,historyLoaded:true,bestCheckpoint:'attempt-1-deadbeefdeadbeef'};\n"
            "(async () => {\n"
            "  const results = [];\n"
            "  for (const scenario of scenarios) {\n"
            "    const saved = {lane:Object.assign({}, base, {phase:scenario.phase}, scenario.cleanup ? {cleanupCheckpoint:scenario.cleanup} : {})};\n"
            "    const calls = [], gates = {};\n"
            "    const state = {get: async k => saved[k], set: async (k,v) => {saved[k]=v;}};\n"
            "    const gate = metric => ({ok:true,results:[{acceptance:{verifyRuns:[{stdout:JSON.stringify({accepted:true,improved:true,checkpoint:'attempt-1-deadbeefdeadbeef',current:{metric,checkpoint:'attempt-1-deadbeefdeadbeef'}})}]}}]});\n"
            "    const measured = {ok:true,output:JSON.stringify({status:'exact',match_percent:100,files_changed:[],prepared_rows:['emi/etc/shop/00@function:func_801DDFB0']})};\n"
            "    const runs = {run: async (k,o) => {calls.push(k); if(o.gate) gates[k]=o.gate; return k === 'baseline' || k === 'final-measure' ? measured : "
            "k === 'checkpoint-baseline' ? gate({match_percent:100,exact:true}) : "
            "k === 'checkpoint-pre-cleanup' ? gate({match_percent:100,exact:true}) : "
            "k === 'load-durable-ledger' ? {ok:true,results:[{acceptance:{verifyRuns:[{stdout:JSON.stringify({entries:[]})}]}}]} : "
            "k.startsWith('record-ledger-') ? {ok:true} : "
            "k === 'integrate' ? {ok:true,results:[{acceptance:{verifyRuns:[{stdout:JSON.stringify({integrated:true,commit:'test'})}]}}]} : "
            "{ok:true,output:JSON.stringify({verdict:'pass',prepared_rows:['emi/etc/shop/00@function:func_801DDFB0']})};}};\n"
            "    async function lane(){\n" + rendered + "\n}\n"
            "    const result = await lane();\n"
            "    results.push({name:scenario.name,gate:gates['restore-interrupted']||'',expect:scenario.expect,status:result.status});\n"
            "  }\n"
            "  console.log(JSON.stringify(results));\n"
            "})();\n"
        )
        crash_results = json.loads(run("node", str(crash)).stdout)
        assert [scenario["name"] for scenario in crash_results] == [
            "final-measure",
            "cleanup-pending",
            "cleanup",
            "cleanup-restore",
            "consolidation-restore",
            "integrate",
        ]
        for scenario in crash_results:
            assert scenario["gate"].endswith(
                "--checkpoint '" + scenario["expect"] + "'"
            ), scenario
            assert scenario["status"] == "integrated", scenario
        ladder = Path(directory) / "ladder.js"
        ladder.write_text(
            "const saved = {}, calls = [];\n"
            "const state = {get: async k => saved[k], set: async (k,v) => {saved[k]=v;}};\n"
            "const gate = (metric,leaf='attempt-1-deadbeefdeadbeef') => ({ok:true,results:[{acceptance:{verifyRuns:[{stdout:JSON.stringify({accepted:true,checkpoint:leaf,current:{metric,checkpoint:leaf}})}]}}]});\n"
            "const result = (rung, extra={}) => ({ok:true,output:JSON.stringify({status:'partial',match_percent:50,files_changed:[],rung,variants_tried:[1,2,3],...extra})});\n"
            "let reviews = 0; const review = () => {reviews++; return {ok:true,output:JSON.stringify({verdict:'needs-fix',prepared_rows:[],experiments:[1,2,3].map(n=>({lever:'e'+reviews+'-'+n,expected_effect:String(n)}))})};};\n"
            "const runs = {run: async (k,o) => {calls.push(k); if(k==='baseline'||k==='final-measure') return {ok:true,output:JSON.stringify({status:'partial',match_percent:50,files_changed:[],prepared_rows:[]})}; if(k==='checkpoint-baseline') return gate({match_percent:50,exact:false}); if(k.startsWith('checkpoint-best-')) {const n=Number(k.slice(16)), s=n===7?60:70; return gate({match_percent:s,exact:false},'attempt-'+(n+1)+'-'+String(n).repeat(16));} if(k==='load-durable-ledger') return {ok:true,results:[{acceptance:{verifyRuns:[{stdout:JSON.stringify({entries:[]})}]}}]}; if(k.startsWith('record-ledger-')) return {ok:true}; if(k.startsWith('reverse-')) {const m=o.task.match(/ladder rung ([a-z-]+)/), rung=m[1]; return result(rung,rung==='compiler-profile'?{coverage_complete:true}:rung==='permuter'?{coordinator_runs:1}:{});} if(k.startsWith('review-')) return o.task.includes('compiler-ceiling')?{ok:true,output:JSON.stringify({verdict:'pass',ladder_exhausted:true})}:review(); if(k==='restore-final') return {ok:true}; return review();}};\n"
            "async function lane(){\n" + rendered + "\n}\n"
            "lane().then(v => console.log(JSON.stringify({result:v,state:saved,calls})));\n"
        )
        ladder_result = json.loads(run("node", str(ladder)).stdout)
        advances = [
            row["rung"]
            for row in ladder_result["result"]["ledger"]
            if row["lever"] == "ladder advance"
        ]
        assert advances == [
            "static-allocation",
            "compiler-profile",
            "permuter",
            "compiler-ceiling",
        ], advances
        assert ladder_result["calls"].count("reverse-7") == 1
        assert (
            len(
                [call for call in ladder_result["calls"] if call.startswith("reverse-")]
            )
            == 9
        )
        assert ladder_result["result"]["status"] == "restored-ladder-exhausted"
        improving = Path(directory) / "ladder-improving.js"
        improving.write_text(
            ladder.read_text().replace(
                "match_percent:50,files_changed:[],rung",
                "match_percent:rung==='compiler-profile'?60:rung==='permuter'?70:50,files_changed:[],rung",
            )
        )
        improving_result = json.loads(run("node", str(improving)).stdout)
        improving_calls = improving_result["calls"]
        assert sum(call.startswith("reverse-7") for call in improving_calls) == 1
        assert sum(call.startswith("reverse-8") for call in improving_calls) == 1
        improving_rungs = [
            row["rung"]
            for row in improving_result["result"]["ledger"]
            if row.get("rung") in ("compiler-profile", "permuter")
            and row["lever"] != "ladder advance"
        ]
        assert (
            improving_rungs.count("compiler-profile") == 1
            and improving_rungs.count("permuter") == 1
        )
        assert improving_result["result"]["bestScore"] == 70
        assert improving_result["result"]["bestCheckpoint"].startswith("attempt-9-")
        assert [
            call for call in improving_calls if call.startswith("checkpoint-best-")
        ] == [
            "checkpoint-best-7",
            "checkpoint-best-8",
        ]
        promotion = Path(directory) / "best-promotion.js"
        promotion.write_text(
            "const mode=process.argv[2], saved={}, calls=[], gates={}; let crash=true, hostBest=null;\n"
            "const clone=v=>JSON.parse(JSON.stringify(v));\n"
            "const state={get:async k=>saved[k]===undefined?undefined:clone(saved[k]),set:async(k,v)=>{if(k==='lane'&&crash&&mode==='after-return'&&v.phase==='ready'&&v.bestScore===60){crash=false;throw new Error('crash after return');}saved[k]=clone(v);if(k==='lane'&&crash&&['before-capture','malformed','mismatch'].includes(mode)&&v.phase==='best-promoting'){crash=false;throw new Error('crash before capture');}}};\n"
            "const gate=(metric,leaf)=>({ok:true,results:[{acceptance:{verifyRuns:[{stdout:JSON.stringify({accepted:true,checkpoint:leaf,current:{metric,checkpoint:leaf}})}]}}]});\n"
            "const bestGate=best=>({ok:true,results:[{acceptance:{verifyRuns:[{stdout:JSON.stringify({best})}]}}]});\n"
            "const measured=n=>({ok:true,output:JSON.stringify({status:'partial',match_percent:n,files_changed:[],prepared_rows:[]})});\n"
            "const runs={run:async(k,o)=>{calls.push(k);if(o.gate)gates[k]=o.gate;if(k==='load-durable-ledger')return {ok:true,results:[{acceptance:{verifyRuns:[{stdout:'{\\\"entries\\\":[]}'}]}}]};if(k.startsWith('record-ledger-'))return {ok:true};if(k==='baseline')return measured(50);if(k==='checkpoint-baseline'){hostBest={selector:'emi/etc/shop/00@0x801DDFB0',attempt:1,checkpoint:'attempt-1-1111111111111111',metric:{match_percent:50,exact:false}};return gate(hostBest.metric,hostBest.checkpoint);}if(k.startsWith('reverse-'))return {ok:true,output:JSON.stringify({status:'partial',match_percent:60,files_changed:[],rung:'clean-c',variants_tried:[1,2,3]})};if(k.startsWith('checkpoint-best-')){if(mode==='failure')return {ok:false};hostBest={selector:'emi/etc/shop/00@0x801DDFB0',attempt:2,checkpoint:'attempt-2-6666666666666666',metric:{match_percent:60,exact:false}};if(mode==='after-host'&&crash){crash=false;throw new Error('crash after durable host capture');}return gate(hostBest.metric,hostBest.checkpoint);}if(k==='inspect-promoted-best'){if(mode==='malformed')return {ok:true,results:[{acceptance:{verifyRuns:[{stdout:'not json'}]}}]};if(mode==='mismatch')return bestGate({...hostBest,attempt:99,metric:{match_percent:60}});return bestGate(hostBest);}if(k.startsWith('review-'))return {ok:true,output:JSON.stringify({verdict:'block',findings:['stop']})};if(k==='final-measure')return measured(55);if(k==='final-review')return {ok:true,output:JSON.stringify({verdict:'pass',prepared_rows:[]})};if(k.startsWith('restore-'))return {ok:true};return {ok:true,output:'{}'};}};\n"
            "async function lane(){\n" + rendered + "\n}\n"
            "(async()=>{let firstError='',secondError='',result=null;try{await lane();}catch(e){firstError=e.message;}if(['before-capture','after-host','after-return','malformed','mismatch'].includes(mode)){try{result=await lane();}catch(e){secondError=e.message;}}console.log(JSON.stringify({firstError,secondError,result,state:saved,calls,gates}));})();\n"
        )
        failed_capture = json.loads(run("node", str(promotion), "failure").stdout)
        assert failed_capture["firstError"] == "best checkpoint capture failed"
        assert failed_capture["state"]["lane"]["phase"] == "best-promoting"
        assert failed_capture["state"]["lane"]["bestScore"] == 50
        for mode, first_error in (
            ("after-host", "crash after durable host capture"),
            ("after-return", "crash after return"),
        ):
            recovered = json.loads(run("node", str(promotion), mode).stdout)
            assert recovered["firstError"] == first_error
            assert recovered["secondError"] == ""
            assert recovered["result"]["bestScore"] == 60
            assert recovered["result"]["bestCheckpoint"] == "attempt-2-6666666666666666"
            restores = [
                recovered["gates"][key]
                for key in recovered["gates"]
                if key == "restore-interrupted"
            ]
            assert restores[-1].endswith("--checkpoint 'attempt-2-6666666666666666'")
            assert recovered["calls"].count("checkpoint-best-1") == 1
            assert recovered["state"]["lane"]["queue"] == []
            assert (
                len(
                    [
                        row
                        for row in recovered["state"]["lane"]["ledger"]
                        if row.get("attempt") == 1
                    ]
                )
                == 1
            )
        before = json.loads(run("node", str(promotion), "before-capture").stdout)
        assert before["firstError"] == "crash before capture"
        assert before["secondError"] == ""
        assert before["result"]["bestScore"] == 60
        assert before["calls"].count("checkpoint-best-1") == 1
        assert (
            len(
                [
                    row
                    for row in before["state"]["lane"]["ledger"]
                    if row.get("attempt") == 1
                ]
            )
            == 1
        )
        malformed = json.loads(run("node", str(promotion), "malformed").stdout)
        assert malformed["secondError"]
        mismatch = json.loads(run("node", str(promotion), "mismatch").stdout)
        assert (
            mismatch["secondError"]
            == "durable best checkpoint mismatches pending promotion"
        )
        state = json.loads(
            run(
                "python3",
                str(SCRIPTS / "lane-worktree.py"),
                "create",
                "--key",
                key,
                "--selector",
                SELECTOR,
                "--allow-dirty",
            ).stdout
        )
        worktree = Path(state["worktree"])
        session_dir = Path(state["session_dir"])
        assert session_dir.is_absolute()
        assert session_dir.is_dir()
        assert session_dir.is_relative_to(ROOT / ".pi-subagents/sessions/lift-loop")
        assert state["launch"] == {
            "cwd": str(worktree),
            "worktree": False,
            "sessionDir": str(session_dir),
            "async": True,
        }
        stale_key = f"orchestration-stale-session-{run_id}"
        stale_session = ROOT / ".pi-subagents/sessions/lift-loop" / stale_key
        stale_session.mkdir(parents=True, exist_ok=True)
        rejected(
            "python3",
            str(SCRIPTS / "lane-worktree.py"),
            "create",
            "--key",
            stale_key,
            "--selector",
            SELECTOR,
            "--allow-dirty",
        )
        stale_session.rmdir()
        marker = worktree / "orchestration-self-check.txt"
        marker.write_text("shared cwd\n")
        manager = str(SCRIPTS / "lane-worktree.py")
        rejected(
            "python3", manager, "export", "--key", key, "--selector", "wrong@0x00000000"
        )
        generated = worktree / "build/orchestration-private.bin"
        generated.parent.mkdir(parents=True, exist_ok=True)
        generated.write_bytes(b"private")
        generated_handoff = json.loads(
            run(
                "python3", manager, "export", "--key", key, "--selector", SELECTOR
            ).stdout
        )
        assert all(
            "build/orchestration-private.bin" not in path
            for path in generated_handoff["changed"]
        )
        generated.unlink()
        handoff = json.loads(
            run(
                "python3", manager, "export", "--key", key, "--selector", SELECTOR
            ).stdout
        )
        assert handoff["base"] == state["base"] and handoff["patch_sha256"]
        assert "orchestration-self-check.txt" in Path(handoff["patch"]).read_text()
        sentinel_key = f"orchestration-unknown-{run_id}"
        sentinel = ROOT.parent / ".bof3-lift-worktrees" / sentinel_key
        sentinel.mkdir(parents=True, exist_ok=True)
        (sentinel / "keep").write_text("safe\n")
        rejected("python3", manager, "remove", "--key", sentinel_key)
        assert (sentinel / "keep").exists()
        (sentinel / "keep").unlink()
        sentinel.rmdir()
    run("python3", str(SCRIPTS / "lane-worktree.py"), "remove", "--key", key)
    assert not session_dir.exists()
    symlink_key = f"orchestration-session-symlink-{run_id}"
    symlink = ROOT / ".pi-subagents/sessions/lift-loop" / symlink_key
    outside = Path(tempfile.mkdtemp())
    symlink.symlink_to(outside, target_is_directory=True)
    rejected(
        "python3",
        str(SCRIPTS / "lane-worktree.py"),
        "create",
        "--key",
        symlink_key,
        "--selector",
        SELECTOR,
        "--allow-dirty",
    )
    symlink.unlink()
    outside.rmdir()
    for owned in (ROOT / "out/lift-loop/handoffs").glob(key + ".*"):
        if owned.is_file() and not owned.is_symlink():
            owned.unlink()
    checkpoint = ROOT / "out/lift-loop/checkpoints" / key
    if checkpoint.is_dir() and not checkpoint.is_symlink():
        import shutil

        shutil.rmtree(checkpoint)
    print("orchestration self-check: OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
