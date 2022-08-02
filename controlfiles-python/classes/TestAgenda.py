import pyarts.arts as cxx
import pyarts

ws = pyarts.workspace.Workspace()
ws.x = [4]

x = cxx.Agenda(ws)

def test(ws):
    ws.x = [2]
    print("in python:", ws.x.value)
    ws.x = [1]

# Note that the actual output order here depends on your stream buffer
# the values counting down is all that matters for
x.add_workspace_method("Print", ws.x, 0)
x.add_workspace_method("VectorSet", ws.x, [3])
x.add_workspace_method("Print", ws.x, 0)
x.add_callback_method(test)
x.add_workspace_method("Print", ws.x, 0)
x.add_workspace_method("Print", "Done!", 0)
x.name = "test_agenda"
x.check(ws)
x.execute(ws)

assert not getattr(ws, "::anon::0").init
assert not getattr(ws, "::anon::1").init
assert not getattr(ws, "::anon::2").init
assert not getattr(ws, "::anon::3").init
assert not getattr(ws, "::anon::4").init
assert not getattr(ws, "::anon::5").init
assert not getattr(ws, "::callback::0").init
