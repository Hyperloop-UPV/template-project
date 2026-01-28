#!/usr/bin/env python3
"""
Pre-flash check script.
Auto-commits uncommitted changes to a temporary branch before flashing
if the binary was compiled with BOARD symbol.

Also handles changes in target submodules (ST-LIB, JSON_ADE) by committing
them to their respective auto-flash-temp branches first.
"""

import subprocess
import sys
import importlib.util
from pathlib import Path
from datetime import datetime

# Constants
TEMP_BRANCH = "auto-flash-temp"

# Submodules to track for auto-commit
TARGET_SUBMODULES = [
    "deps/ST-LIB",
    "Core/Inc/Code_generation/JSON_ADE",
]


def get_script_dir() -> Path:
    return Path(__file__).parent.resolve()


def get_workspace_dir() -> Path:
    return get_script_dir().parent


def check_board_symbol(build_dir: Path) -> bool:
    """Check if the binary was compiled with BOARD symbol."""
    marker_file = build_dir / "board_build_marker"
    return marker_file.exists()


def has_uncommitted_changes(repo_path: Path, ignore_submodules: bool = False) -> bool:
    """Check if there are uncommitted changes in the repository."""
    cmd = ["git", "status", "--porcelain"]
    if ignore_submodules:
        cmd.append("--ignore-submodules")
    result = subprocess.run(
        cmd,
        cwd=repo_path,
        capture_output=True,
        text=True
    )
    return bool(result.stdout.strip())


# -----------------------------------------------------------------------------
# Git Helper Functions
# -----------------------------------------------------------------------------

def run_git(repo_path: Path, args: list, check: bool = True) -> subprocess.CompletedProcess:
    """Run a git command and return the result."""
    result = subprocess.run(
        ["git"] + args,
        cwd=repo_path,
        capture_output=True,
        text=True
    )
    if check and result.returncode != 0:
        raise RuntimeError(f"Git command failed: git {' '.join(args)}\n{result.stderr}")
    return result


def get_current_ref(repo_path: Path) -> tuple:
    """
    Get current branch name or commit hash.
    Returns (ref, is_branch) where is_branch is False if detached HEAD.
    """
    result = run_git(repo_path, ["rev-parse", "--abbrev-ref", "HEAD"])
    ref = result.stdout.strip()
    if ref == "HEAD":
        # Detached HEAD - get the commit hash instead
        result = run_git(repo_path, ["rev-parse", "HEAD"])
        return result.stdout.strip(), False
    return ref, True


def get_current_commit(repo_path: Path) -> str:
    """Get the current commit hash."""
    result = run_git(repo_path, ["rev-parse", "HEAD"])
    return result.stdout.strip()


def branch_exists(repo_path: Path, branch_name: str) -> bool:
    """Check if a branch exists locally."""
    result = run_git(repo_path, ["branch", "--list", branch_name], check=False)
    return bool(result.stdout.strip())


def stash_changes(repo_path: Path) -> bool:
    """Stash all changes including untracked files. Returns True if something was stashed."""
    if not has_uncommitted_changes(repo_path):
        return False
    result = run_git(repo_path, ["stash", "push", "-u", "-m", "auto-flash-temp-stash"])
    return "No local changes to save" not in result.stdout


def pop_stash(repo_path: Path) -> bool:
    """Pop the most recent stash. Returns True on success, False if conflicts."""
    result = run_git(repo_path, ["stash", "pop"], check=False)
    if result.returncode != 0:
        return False
    # Check for merge conflicts after pop
    status_result = run_git(repo_path, ["status", "--porcelain"], check=False)
    # If any line starts with "U" or has "UU", there are conflicts
    for line in status_result.stdout.splitlines():
        if line.startswith("U") or "UU" in line[:2]:
            return False
    return True


def abort_merge(repo_path: Path) -> None:
    """Abort any in-progress merge and reset to clean state."""
    run_git(repo_path, ["merge", "--abort"], check=False)
    run_git(repo_path, ["reset", "--hard", "HEAD"], check=False)


def has_merge_conflicts(repo_path: Path) -> bool:
    """Check if there are unmerged paths (merge conflicts)."""
    result = run_git(repo_path, ["diff", "--name-only", "--diff-filter=U"], check=False)
    return bool(result.stdout.strip())


def checkout_ref(repo_path: Path, ref: str, is_branch: bool = True) -> None:
    """Checkout a branch or commit (detached HEAD)."""
    if is_branch:
        run_git(repo_path, ["checkout", ref])
    else:
        run_git(repo_path, ["checkout", "--detach", ref])


def create_branch(repo_path: Path, branch_name: str, start_point: str = "HEAD") -> None:
    """Create a new branch at the given start point."""
    run_git(repo_path, ["branch", branch_name, start_point])


def commit_all_changes(repo_path: Path, message: str) -> str | None:
    """Stage and commit all changes. Returns the new commit hash, or None if nothing to commit."""
    run_git(repo_path, ["add", "-A"])
    # Unstage the metadata file - it's regenerated after commit and shouldn't be part of it
    metadata_file = "Core/Src/Runes/generated_metadata.cpp"
    run_git(repo_path, ["reset", "HEAD", "--", metadata_file], check=False)
    # Check if there are staged changes to commit
    result = run_git(repo_path, ["diff", "--cached", "--quiet"], check=False)
    if result.returncode == 0:
        # No staged changes - nothing to commit
        return None
    run_git(repo_path, ["commit", "-m", message])
    return get_current_commit(repo_path)


def push_to_remote(repo_path: Path, branch_name: str) -> bool:
    """Push branch to origin. Returns True on success, False on failure.
    
    If push is rejected due to non-fast-forward (diverged history), 
    fetches remote and rebases local commit on top. This preserves
    history of previous auto-flash commits while keeping a linear history.
    """
    # 1. Try normal push first
    result = run_git(repo_path, ["push", "-u", "origin", branch_name], check=False)
    if result.returncode == 0:
        return True
    
    # 2. Check if rejected due to non-fast-forward
    if "non-fast-forward" not in result.stderr and "rejected" not in result.stderr:
        return False  # Some other error (e.g., no network)
    
    # 3. Fetch remote branch
    print(f"  Push rejected (diverged history). Fetching and rebasing...")
    fetch_result = run_git(repo_path, ["fetch", "origin", branch_name], check=False)
    if fetch_result.returncode != 0:
        print(f"  Failed to fetch remote branch.")
        return False
    
    # 4. Rebase local commit onto remote (keeps linear history, no merge commits)
    rebase_result = run_git(repo_path, ["rebase", f"origin/{branch_name}"], check=False)
    if rebase_result.returncode != 0:
        # Rebase conflict - abort and give up
        print(f"  Rebase conflict with remote. Aborting rebase.")
        run_git(repo_path, ["rebase", "--abort"], check=False)
        return False
    
    print(f"  Rebased successfully onto remote.")
    
    # 5. Retry push
    retry_result = run_git(repo_path, ["push", "-u", "origin", branch_name], check=False)
    return retry_result.returncode == 0


# -----------------------------------------------------------------------------
# Submodule Functions
# -----------------------------------------------------------------------------

def submodule_has_changes(workspace_dir: Path, submodule_path: str) -> bool:
    """Check if a specific submodule has uncommitted changes."""
    submodule_full_path = workspace_dir / submodule_path
    if not submodule_full_path.exists():
        return False
    return has_uncommitted_changes(submodule_full_path)


def get_submodules_with_changes(workspace_dir: Path) -> list:
    """Return list of target submodules that have uncommitted changes."""
    changed = []
    for submodule_path in TARGET_SUBMODULES:
        if submodule_has_changes(workspace_dir, submodule_path):
            changed.append(submodule_path)
    return changed


def auto_commit_submodule(workspace_dir: Path, submodule_path: str, parent_branch: str) -> dict:
    """
    Auto-commit changes in a submodule to its auto-flash-temp branch.
    Returns dict with original_ref, is_branch, and commit_hash.
    
    NOTE: Leaves the submodule on auto-flash-temp branch (caller must restore later).
    """
    submodule_full_path = workspace_dir / submodule_path
    original_ref, is_branch = get_current_ref(submodule_full_path)
    original_commit = get_current_commit(submodule_full_path)
    stash_created = False
    on_temp_branch = False
    commit_hash = None

    try:
        # 1. Stash changes
        print(f"  Stashing changes...")
        stash_created = stash_changes(submodule_full_path)

        # 2. Checkout/create temp branch - always reset to original_commit
        # History preservation happens at push time via pull-and-merge
        if branch_exists(submodule_full_path, TEMP_BRANCH):
            print(f"  Checking out existing branch '{TEMP_BRANCH}'...")
            checkout_ref(submodule_full_path, TEMP_BRANCH, is_branch=True)
            on_temp_branch = True
            # Reset to current HEAD - merge will happen at push time if needed
            print(f"  Resetting to current HEAD ({original_commit[:8]})...")
            run_git(submodule_full_path, ["reset", "--hard", original_commit])
        else:
            print(f"  Creating new branch '{TEMP_BRANCH}'...")
            create_branch(submodule_full_path, TEMP_BRANCH, original_commit)
            checkout_ref(submodule_full_path, TEMP_BRANCH, is_branch=True)
            on_temp_branch = True

        # 3. Pop stash to apply changes on temp branch
        # Since we reset to the same commit we stashed from, this should always work
        if stash_created:
            print(f"  Applying changes to temp branch...")
            if not pop_stash(submodule_full_path):
                raise RuntimeError(f"Failed to apply stashed changes to {submodule_path}")
            stash_created = False  # Stash was consumed

        # 4. Commit all changes
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        commit_msg = f"Auto-flash commit from parent '{parent_branch}' @ {timestamp}"
        print(f"  Committing changes...")
        commit_hash = commit_all_changes(submodule_full_path, commit_msg)
        
        if commit_hash is None:
            # No changes to commit - use current HEAD (changes already match temp branch)
            print(f"  No new changes to commit (already up to date with temp branch)")
            commit_hash = get_current_commit(submodule_full_path)
            push_succeeded = True  # Nothing new to push
        else:
            print(f"  Created commit: {commit_hash[:8]}")

            # 5. Attempt to push
            print(f"  Pushing to origin/{TEMP_BRANCH}...")
            push_succeeded = push_to_remote(submodule_full_path, TEMP_BRANCH)

            if not push_succeeded:
                print(f"  WARNING: Could not push to remote. Commit {commit_hash[:8]} stored locally.")
            else:
                print(f"  Successfully pushed to origin/{TEMP_BRANCH}")

        # NOTE: We leave the submodule on auto-flash-temp so the parent repo
        # sees the updated submodule pointer. Caller must restore later.

        return {
            "original_ref": original_ref,
            "is_branch": is_branch,
            "commit_hash": commit_hash,
            "push_succeeded": push_succeeded,
        }

    except Exception as e:
        # On error, try to restore the submodule to original state
        if on_temp_branch:
            try:
                checkout_ref(submodule_full_path, original_ref, is_branch)
            except Exception:
                pass
        raise e


def restore_submodule(workspace_dir: Path, submodule_path: str, info: dict) -> None:
    """Restore a submodule to its original state after parent commit."""
    submodule_full_path = workspace_dir / submodule_path
    original_ref = info["original_ref"]
    is_branch = info["is_branch"]
    commit_hash = info["commit_hash"]

    print(f"  Restoring submodule: {submodule_path}")

    # 1. Checkout original ref (branch or detached HEAD)
    checkout_ref(submodule_full_path, original_ref, is_branch)

    # 2. Restore uncommitted changes by cherry-picking
    if commit_hash:
        run_git(submodule_full_path, ["cherry-pick", "--no-commit", commit_hash], check=False)
        run_git(submodule_full_path, ["reset", "HEAD"], check=False)


# -----------------------------------------------------------------------------
# Metadata Generation
# -----------------------------------------------------------------------------

def run_metadata_generation(workspace_dir: Path) -> None:
    """Run the metadata generation script by importing and calling it directly."""
    metadata_script = workspace_dir / "tools" / "generate_binary_metadata.py"
    spec = importlib.util.spec_from_file_location("generate_binary_metadata", metadata_script)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Could not load metadata generation script: {metadata_script}")
    metadata_module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(metadata_module)
    metadata_module.main()


# -----------------------------------------------------------------------------
# Main Auto-Commit Logic
# -----------------------------------------------------------------------------

def auto_commit_to_temp_branch(workspace_dir: Path, submodule_paths_to_stage: list | None = None) -> tuple:
    """
    Auto-commit changes to the temp branch.
    
    Args:
        workspace_dir: Path to the workspace/repository
        submodule_paths_to_stage: List of submodule paths to stage before committing
                                  (their pointers were updated by submodule auto-commits)
    
    Returns (commit_hash, push_succeeded).
    """
    if submodule_paths_to_stage is None:
        submodule_paths_to_stage = []

    original_ref, is_branch = get_current_ref(workspace_dir)
    original_commit = get_current_commit(workspace_dir)
    stash_created = False
    on_temp_branch = False
    commit_hash = None

    try:
        # 0. Stage submodule pointer updates BEFORE stashing
        # This ensures the stash includes the staged submodule pointers
        if submodule_paths_to_stage:
            print("Staging submodule pointer updates...")
            for submodule_path in submodule_paths_to_stage:
                run_git(workspace_dir, ["add", submodule_path])

        # 1. Stash current changes (now includes staged submodule pointers)
        print("Stashing current changes...")
        stash_created = stash_changes(workspace_dir)

        # 2. Handle temp branch - always reset to original_commit
        # History preservation happens at push time via pull-and-merge
        if branch_exists(workspace_dir, TEMP_BRANCH):
            print(f"Checking out existing branch '{TEMP_BRANCH}'...")
            checkout_ref(workspace_dir, TEMP_BRANCH, is_branch=True)
            on_temp_branch = True
            # Reset to current HEAD - merge will happen at push time if needed
            print(f"Resetting to current HEAD ({original_commit[:8]})...")
            run_git(workspace_dir, ["reset", "--hard", original_commit])
        else:
            print(f"Creating new branch '{TEMP_BRANCH}'...")
            create_branch(workspace_dir, TEMP_BRANCH, original_commit)
            checkout_ref(workspace_dir, TEMP_BRANCH, is_branch=True)
            on_temp_branch = True

        # 3. Pop stash to apply changes on temp branch
        # Since we reset to the same commit we stashed from, this should always work
        if stash_created:
            print("Applying changes to temp branch...")
            if not pop_stash(workspace_dir):
                raise RuntimeError("Failed to apply stashed changes to temp branch")
            stash_created = False  # Stash was consumed

        # 4. Commit all changes
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        commit_msg = f"Auto-flash commit from '{original_ref}' @ {timestamp}"
        print("Committing changes...")
        commit_hash = commit_all_changes(workspace_dir, commit_msg)
        
        if commit_hash is None:
            # No changes to commit - this shouldn't happen for main repo
            # since we checked for changes earlier, but handle it gracefully
            print("No new changes to commit (already up to date with temp branch)")
            commit_hash = get_current_commit(workspace_dir)
            push_succeeded = True
        else:
            print(f"Created commit: {commit_hash[:8]}")

            # 5. Run metadata generation (to capture the commit hash)
            # Note: metadata file is gitignored, so it won't affect the commit
            print("Regenerating binary metadata...")
            run_metadata_generation(workspace_dir)

            # 6. Attempt to push
            print(f"Pushing to origin/{TEMP_BRANCH}...")
            push_succeeded = push_to_remote(workspace_dir, TEMP_BRANCH)

            if not push_succeeded:
                print(f"WARNING: Could not push to remote. Commit {commit_hash[:8]} stored locally.")
                print("         It will be pushed automatically on next flash with connectivity.")
            else:
                print(f"Successfully pushed to origin/{TEMP_BRANCH}")

        return commit_hash, push_succeeded

    finally:
        # 7. Always return to original branch/ref
        if on_temp_branch:
            print(f"Returning to '{original_ref}'...")
            # Use --force to overwrite the gitignored metadata file
            run_git(workspace_dir, ["checkout", "--force", original_ref] if is_branch 
                    else ["checkout", "--force", "--detach", original_ref])

        # 8. Regenerate metadata with the temp branch commit hash
        # Save the content to restore after cherry-pick (which would fail if file exists)
        metadata_file = workspace_dir / "Core" / "Src" / "Runes" / "generated_metadata.cpp"
        metadata_content = None
        if commit_hash:
            print("Regenerating metadata with flash commit...")
            run_git(workspace_dir, ["checkout", "--detach", commit_hash])
            run_metadata_generation(workspace_dir)
            # Save metadata content
            metadata_content = metadata_file.read_text() if metadata_file.exists() else None
            # Delete file so cherry-pick can work
            if metadata_file.exists():
                metadata_file.unlink()
            # Return to original ref
            run_git(workspace_dir, ["checkout", original_ref] if is_branch 
                    else ["checkout", "--detach", original_ref])

        # 9. Restore uncommitted changes on original branch
        # The changes are no longer in stash (we committed them on temp branch)
        # We need to re-create the working directory state by cherry-picking
        if commit_hash:
            print("Restoring uncommitted changes on original branch...")
            # Cherry-pick the diff from temp branch without committing
            run_git(workspace_dir, ["cherry-pick", "--no-commit", commit_hash], check=False)
            # Reset the index but keep changes in working directory
            run_git(workspace_dir, ["reset", "HEAD"], check=False)
            
        # 10. Restore metadata file with correct commit hash (after cherry-pick)
        if metadata_content:
            metadata_file.write_text(metadata_content)


# -----------------------------------------------------------------------------
# Main Entry Point
# -----------------------------------------------------------------------------

def main():
    workspace_dir = get_workspace_dir()

    # Default build directory - can be overridden by command line argument
    if len(sys.argv) > 1:
        build_dir = Path(sys.argv[1])
    else:
        build_dir = workspace_dir / "out" / "build"

    print(f"Workspace directory: {workspace_dir}")
    print(f"Build directory: {build_dir}")

    # Check if this is a BOARD build
    if not check_board_symbol(build_dir):
        print("Binary was not compiled with BOARD symbol (or marker not found). Skipping pre-flash check.")
        sys.exit(0)

    print("Binary was compiled with BOARD symbol. Checking for uncommitted changes...")

    # Check for changes in main repo (excluding submodules)
    main_repo_has_changes = has_uncommitted_changes(workspace_dir, ignore_submodules=True)

    # # Check for changes in target submodules
    # submodules_with_changes = get_submodules_with_changes(workspace_dir)

    # If no changes anywhere, proceed with flash
    # if not main_repo_has_changes and not submodules_with_changes:
    if not main_repo_has_changes:
        print("No uncommitted changes. Proceeding with flash.")
        sys.exit(0)

    print("Uncommitted changes detected. Auto-committing to temp branch...")
    print("-" * 50)

    # Track submodule info for restoration later
    # submodule_info = {}  # {path: {"original_ref": ..., "is_branch": ..., "commit_hash": ...}}

    try:
        # Get parent branch name for submodule commit messages
        # parent_ref, _ = get_current_ref(workspace_dir)

        # # Step 1: Process submodules first (if any have changes)
        # for submodule_path in submodules_with_changes:
        #     print(f"\n--- Processing submodule: {submodule_path} ---")
        #     info = auto_commit_submodule(workspace_dir, submodule_path, parent_ref)
        #     submodule_info[submodule_path] = info

        # Step 2: Process main repo
        # Pass the list of submodule paths that need their pointers staged
        print(f"\n--- Processing main repository ---")
        # submodule_paths = list(submodule_info.keys()) if submodule_info else []
        commit_hash, push_succeeded = auto_commit_to_temp_branch(
            workspace_dir,
            # submodule_paths_to_stage=submodule_paths
        )

        # # Step 3: Restore submodules to their original state
        # if submodule_info:
        #     print(f"\n--- Restoring submodules ---")
        #     for submodule_path, info in submodule_info.items():
        #         restore_submodule(workspace_dir, submodule_path, info)

        # Print summary
        print("-" * 50)
        print(f"Changes committed to '{TEMP_BRANCH}' as {commit_hash[:8]}")
        # if submodule_info:
        #     submodule_summary = ", ".join(
        #         f"{path.split('/')[-1]} ({info['commit_hash'][:8]})"
        #         for path, info in submodule_info.items()
        #     )
        #     print(f"Submodules committed: {submodule_summary}")
        if push_succeeded:
            print(f"Pushed to remote: origin/{TEMP_BRANCH}")
        else:
            print("WARNING: Changes not pushed (no connectivity). Will retry on next flash.")
        print("Proceeding with flash...")

    except Exception as e:
        print(f"ERROR during auto-commit: {e}")
        # # Try to restore submodules even on error
        # for submodule_path, info in submodule_info.items():
        #     try:
        #         restore_submodule(workspace_dir, submodule_path, info)
        #     except Exception:
        #         print(f"WARNING: Could not restore submodule {submodule_path}")
        print("Flash aborted. Please resolve the issue manually.")
        sys.exit(1)

    sys.exit(0)


if __name__ == "__main__":
    main()
