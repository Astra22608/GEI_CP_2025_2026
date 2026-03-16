- module (ring).

- export([start/1, send/3, stop/1]).

start(N) ->
    Pids = crear_anillo(N, 0),
    conectar_circular(Pids, hd(Pids)),
    hd(Pids).

crear_anillo(0,_) -> [];
crear_anillo(N, Id) ->
   [spawn(fun() -> loop(Id, nil) end) | crear_anillo(N-1, Id+1)].

conectar_circular([Pid], FirstPid) -> Pid ! {set_next, FirstPid};
conectar_circular([Pid1, Pid2 | Rest], FirstPid) ->
    Pid1 ! {set_next, Pid2},
    conectar_circular([Pid2 | Rest], FirstPid).

send(Pid, N, Msg) ->
    Pid ! {send, N, Msg},
    ok.

stop(Pid) ->
    Pid ! stop,
    ok.

loop(Id, Next) ->
    receive
        {set_next, N} -> 
            loop(Id, N);
        {send, N, Msg} ->
            %% Formato exacto según el PDF: "Id receiving message Msg with N-1 left"
            io:format("~p receiving message ~p with ~p left~n", [Id, Msg, N-1]),
            if N > 1 -> Next ! {send, N-1, Msg}; true -> ok end,
            loop(Id, Next);
        {stop, FirstPid} ->
            if Next /= FirstPid -> Next ! {stop, FirstPid}; true -> ok end
    end.