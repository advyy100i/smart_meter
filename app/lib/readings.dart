import 'dart:convert';

import 'package:flutter/material.dart';
import 'package:web_socket_channel/web_socket_channel.dart';
import 'package:web_socket_channel/status.dart' as status;

class ReadingsPage extends StatefulWidget {
  final String meterId;
  const ReadingsPage({super.key, required this.meterId});

  @override
  State<StatefulWidget> createState() => _ReadingsPageState();
}

class _ReadingsPageState extends State<ReadingsPage> {
  // websocket channele for websocket connection
  late WebSocketChannel _channel;

  // stores the readings fetched from websocket server
  double _voltage = 0.0;
  double _current = 0.0;
  double _power = 0.0;
  double _energy = 0.0;

  // stores any error while fetching data from websocket server
  String _error = '';

  @override
  void initState() {
    super.initState();

    final Uri uri = Uri.parse('ws://localhost:8080/');
    _channel = WebSocketChannel.connect(uri);

    _channel.sink.add(widget.meterId);
    _channel.stream.listen(
      (data) {
        final decodedData = jsonDecode(data);
        setState(() {
          _voltage = decodedData['voltage'];
          _current = decodedData['current'];
          _power = decodedData['power'];
          _energy = decodedData['energy'];
        });
      },
      onError: (error) {
        setState(() {
          _error = error.toString();
        });
      },
      onDone: () {},
    );
  }

  @override
  void dispose() {
    _channel.sink.close(status.goingAway);
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text("Readings"),
      ),
      body: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          children: [
            Text('Voltage: ${_voltage.toStringAsFixed(2)}'),
            Text('Current: ${_current.toStringAsFixed(2)}'),
            Text('Power: ${_power.toStringAsFixed(2)}'),
            Text('Energy: ${_energy.toStringAsFixed(2)}'),
            if (_error.isNotEmpty) Text('Error: $_error'),
          ],
        ),
      ),
    );
  }
}
