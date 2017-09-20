import { InterfacePage } from './app.po';

describe('interface App', () => {
  let page: InterfacePage;

  beforeEach(() => {
    page = new InterfacePage();
  });

  it('should display welcome message', () => {
    page.navigateTo();
    expect(page.getParagraphText()).toEqual('Welcome to app!');
  });
});
