#!/usr/bin/env python3
from datetime import datetime
from pathlib import Path
import os
import re

from git import Repo
from jinja2 import Environment, FileSystemLoader

MACRO_PATTERN = re.compile(r"ADD_TO_FLASH_INFO\(([^)]+)\)")
ACCEPTED_EXTENSIONS = {".cpp", ".c", ".h", ".hpp"}


class Variable:
    def __init__(self, name: str, value: str):
        self.name = name
        self.value = value


def get_git_root() -> Path:
    repo_root = Path(__file__).resolve()
    while not (repo_root / ".git").exists() and repo_root != repo_root.parent:
        repo_root = repo_root.parent
    if not (repo_root / ".git").exists():
        raise RuntimeError("Could not find repository root (.git)")
    return repo_root


def get_current_commit(path: Path) -> str:
    try:
        repo = Repo(path, search_parent_directories=True)
        return repo.head.commit.hexsha[:8]
    except Exception:
        return "--------"


def parse_file(file_path: Path, variables: list[Variable]) -> None:
    with file_path.open("r", encoding="utf-8", errors="ignore") as file:
        for line in file:
            match = MACRO_PATTERN.search(line)
            if not match:
                continue

            parts = [part.strip() for part in match.group(1).split(",")]
            if len(parts) != 3:
                continue

            var_name = parts[1]
            value = parts[2]
            if var_name == "VAR_NAME" and value == "VALUE":
                continue
            variables.append(Variable(var_name, value))


def search_core_sources(core_root: Path, variables: list[Variable]) -> None:
    for folder_name, _subfolders, filenames in os.walk(core_root):
        for filename in filenames:
            path = Path(folder_name) / filename
            if path.suffix in ACCEPTED_EXTENSIONS:
                parse_file(path, variables)


def main() -> None:
    print("Starting binary metadata generation")
    repo_root = get_git_root()

    variables: list[Variable] = []
    search_core_sources(repo_root / "Core", variables)

    environment = Environment(loader=FileSystemLoader(str(repo_root / "tools")))
    template = environment.get_template("binary_metadata_template.cpp")

    iso_time = datetime.now().strftime("%Y%m%dT%H%M%S")
    stlib_commit = get_current_commit(repo_root / "deps/ST-LIB")
    adj_commit = get_current_commit(repo_root / "deps/adj")
    board_commit = get_current_commit(repo_root)

    output_file = repo_root / "Core/Src/Runes/generated_metadata.cpp"
    content = template.render(
        DateTimeISO8601=iso_time,
        STLIB_COMMIT=stlib_commit,
        ADJ_COMMIT=adj_commit,
        BOARD_COMMIT=board_commit,
        variables=variables,
    )
    output_file.write_text(content, encoding="utf-8")
    print("Generation completed")


if __name__ == "__main__":
    main()
