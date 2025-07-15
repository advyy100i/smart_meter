import 'package:app/readings.dart';
import 'package:flutter/material.dart';

class HomePage extends StatefulWidget {
  const HomePage({super.key});

  @override
  State<StatefulWidget> createState() => _HomePageState();
}

class _HomePageState extends State<HomePage> {
  final TextEditingController _meterIdController = TextEditingController();

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text("Details"),
      ),
      body: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          children: [
            TextField(
              controller: _meterIdController,
              decoration: const InputDecoration(
                label: Text('Meter ID'),
              ),
            ),
            ElevatedButton(
              onPressed: () {
                final meterId = _meterIdController.text;
                if (meterId.isNotEmpty) {
                  Navigator.push(
                    context,
                    MaterialPageRoute(
                      builder: (context) => ReadingsPage(meterId: meterId),
                    ),
                  );
                }
              },
              child: const Text("Connect"),
            )
          ],
        ),
      ),
    );
  }
}
