import { Component } from '@angular/core';
import { HttpClient } from '@angular/common/http';

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.css']
})
export class AppComponent {
  prices = [];
  status: string = null;
  rates: any;
  scanning = false;

  constructor(private http: HttpClient) {}

  ngOnInit() {
    this.status = 'Fetching latest exchange rates...'
    this.http.get(
      'http://api.fixer.io/latest?base=USD').subscribe(rates => {
        this.rates = rates['rates'];
        this.status = 'Start by clicking start';
      });
  }

  scan() {
    this.prices = [];
    this.status = 'Scanning...';
    this.scanning = true;
    this.http.get('http://localhost:1234/').subscribe(prices => {
      this.scanning = false;
      if (prices['prices'].length > 0) {
        this.status = null;
        this.prices = prices['prices'].map(item => {
          if (item['currency'] in this.rates) {
            item['conversion'] = item['amount']/this.rates[item['currency']];
          } else {
            item['conversion'] = null;
          }
          return item;
        });
      } else {
        this.status = 'No prices found';
      }
    });
  }
}
